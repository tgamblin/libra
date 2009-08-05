#include "sampling.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

#include "stl_utils.h"
#include "synchronize_keys.h"
#include "effort_key.h"
#include "ltqnorm.h"

#define USE_MPI
#include "sprng_cpp.h"

#define SEED 985456376

namespace effort {

  sampling_module::sampling_module(size_t initial_sample_size)
    : comm(MPI_COMM_NULL), 
      enabled(false), 
      proportion(0), 
      confidence(0), 
      error(0),
      windows(0),
      windows_per_update(1),
      stats(false),
      trace(true),
      rng(new LCG()),
      initial_sample(initial_sample_size)
  { }


  void sampling_module::init(MPI_Comm comm, double confidence, double error, std::string output_dir) {
    this->comm = comm;
    this->confidence = confidence;
    this->error = error;

    int rank, size;
    PMPI_Comm_rank(comm, &rank);
    PMPI_Comm_size(comm, &size);

    // init random number generator across procsses
    rng->init_sprng(rank, size, SEED, SPRNG_DEFAULT);

    // use first random number to figure out who's enabled to start with.
    if (initial_sample < (size_t)size) {
      initial_sample = size;
    }
    proportion = initial_sample / (double)size;
    enabled = (rng->sprng() < proportion);

    // make ampl log dir
    ostringstream trace_dirname;
    trace_dirname << output_dir << "/ampl";
    trace_dir = trace_dirname.str();
    mkdir(trace_dir.c_str(), 0750);  // create trace directory

    trace_dirname << "/log." << rank;   // create name of local trace file.
    trace_filename = trace_dirname.str();

    // create summary file
    if (rank == 0) {
      ostringstream summary_filename;
      summary_filename << output_dir << "/summary";
      summary_file.open(summary_filename.str().c_str());
    }
  }


  sampling_module::~sampling_module() {
    if (rng) {
      rng->free_sprng(); 
      rng = NULL;
    }
  }


  void sampling_module::set_windows_per_update(size_t wpu) {
    windows_per_update = wpu;
  }


  void sampling_module::set_normalized_error(bool normalized) {
    normalized_error = normalized;
  }


  void sampling_module::set_stats(bool stats) {
    this->stats = stats;
  }


  void sampling_module::set_trace(bool trace) {
    this->trace = trace;
  }


  sample_desc sampling_module::compute_sample_size(double sum, double sum2, size_t N, double confidence, double error) {
    double mean = sum/N;                            // sample mean
    double variance = (sum2/N - (mean*mean));       // estimate w/sample mean
    double stdDev = sqrt(variance);

    if (stdDev < 1e-9) stdDev = 1e-9;                // in case variance is 0.

    // calculate min sample size
    double Za = computeConfidenceInterval(confidence);   // double-tailed norm conf. interval
    double d = error;
    if (normalized_error) d *= mean;           // if normalized, error is a %, so multiply by mean.

    double V = (d/(Za*stdDev));

    size_t min_sample_size = llround(N / (1 + N * V*V));

    return sample_desc(mean, variance, stdDev, min_sample_size);
  }

  struct guide_check {
    set<effort_key>& guide;
    guide_check(set<effort_key>& g) : guide(g) { }

    bool operator()(const effort_key& key) {
      if (guide.empty()) return true;   // empty set => all true.
      effort_key normalized(Metric::time(), 0, key.start_path, key.end_path);
      return (guide.find(normalized) != guide.end());
    }
  };


  void sampling_module::get_sample_keys(effort_data& log, vector<effort_key>& sorted_keys) {
    sorted_keys.clear();

    // Dump keys into the vector.
    sorted_keys.reserve(log.size());
    transform(log.begin(), log.end(), back_inserter(sorted_keys), get_first());

    // remove non-guiding keys.
    sorted_keys.erase(
      partition(sorted_keys.begin(), sorted_keys.end(), guide_check(guide)),
      sorted_keys.end()
    );
  }

  /*
  static void dump(vector<effort_key>& keys) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    ostringstream msg;
    msg << "dumping " << keys.size() << " keys" << endl;
    cerr << msg.str();

    ostringstream filename;
    filename << "keys." << rank;
    string file_str(filename.str());
    ofstream file(file_str.c_str());

    for (size_t i=0; i < keys.size(); i++) {
      file << keys[i] << endl;
    }
  }
  */

  double sampling_module::compute_sample_proportion(effort_data& log) {
    int rank, size;
    PMPI_Comm_rank(comm, &rank);
    PMPI_Comm_size(comm, &size);

    // sync up keys first.
    synchronize_effort_keys(log, comm);

    // Vector to hold keys in identical order across processes
    vector<effort_key> sorted_keys;
    get_sample_keys(log, sorted_keys);

    // Sort vector using heavy key comparison (cmpares by all frames, full module names, offsets)
    sort(sorted_keys.begin(), sorted_keys.end(), effort_key_full_lt());

    vector<sample_desc> vars;
    if (rank == 0) vars.resize(sorted_keys.size());
    size_t voff = 0;

    size_t local_max_sample_size = 0;
    size_t k = 0;
    while (k < sorted_keys.size()) {
      // reduce per-key sum and sum squares to different processes
      double sum, sum2;

      int root;
      for (root=0; root < size && k < sorted_keys.size(); root++, k++) {
        effort_key key = sorted_keys[k];
        double val  = log[key].current;
        double val2 = val * val;

        // compute sum and sum of squares
        PMPI_Reduce(&val,  &sum,  1, MPI_DOUBLE, MPI_SUM, root, comm);
        PMPI_Reduce(&val2, &sum2, 1, MPI_DOUBLE, MPI_SUM, root, comm);
      }

      sample_desc sd;
      if (rank < root) {
        // calculate local sample size and take the max of sizes seen so far.
        sd = compute_sample_size(sum, sum2, size, confidence, error);
        local_max_sample_size = max(sd.sample_size, local_max_sample_size);
      }

      // gather sample_descs to proc 0
      if (stats) {
        MPI_Comm gather_comm;
        PMPI_Comm_split(comm, (rank < root ? 0 : 1), rank, &gather_comm);

        if (rank < root) {
          PMPI_Gather(&sd,         sizeof(sample_desc), MPI_BYTE,
                      &vars[voff], sizeof(sample_desc), MPI_BYTE,
                      0, gather_comm);
        }
        voff += size;

        PMPI_Comm_free(&gather_comm);
      }
    }

    if (stats) {
      key_stats.clear();
      for (size_t i=0; i < vars.size(); i++) {
        key_stats[sorted_keys[i]] = vars[i];
      } 
    }

    // find global sample size with allreduce.
    size_t max_sample_size;
    PMPI_Allreduce(&local_max_sample_size, &max_sample_size, 1, MPI_SIZE_T, MPI_MAX, comm);

    // in case there's really no variance.
    if (max_sample_size < 1) max_sample_size = 1;

    return max_sample_size / (double)size;
  }


  struct variance_gt {
    const stat_map& stats;
    variance_gt(const stat_map& sm) : stats(sm) { }
    bool operator()(const effort_key& lhs, const effort_key& rhs) {
      return stats.find(lhs)->second.variance > stats.find(rhs)->second.variance;
    }
  };

  void sampling_module::sample_step(effort_data& log) { 
    int rank, size;
    PMPI_Comm_rank(comm, &rank);
    PMPI_Comm_size(comm, &size);
    
    if (enabled && trace) {
      // open trace file for writing if needed.
      if (!trace_file.is_open()) {
        trace_file.open(trace_filename.c_str(), ios::app);
      }

      log.write_current_step(trace_file);
    }
    
    if (windows % windows_per_update) {
      proportion = compute_sample_proportion(log);

      if (rank == 0) {
        ostringstream summary;
        summary << "STEP " << log.progress_count << endl;
        summary << "    SampleSize " << (size_t)(proportion * size) << endl;
        summary << "    Proportion " << proportion << endl;
        summary << "    Keys       " << log.size() << endl;

        if (stats) {
          // stats are in vars vector if stats is on.
          size_t place = 1;

          vector<effort_key> vsorted_keys;
          get_sample_keys(log, vsorted_keys);
          sort(vsorted_keys.begin(), vsorted_keys.end(), variance_gt(key_stats));

          for (size_t i=0; i < vsorted_keys.size(); i++) {
            const effort_key& key = vsorted_keys[i];

            
            const sample_desc& sd = key_stats[key];

            summary << setw(4)  << place++
                    << setw(10) << setprecision(3) << sd.mean
                    << setw(10) << setprecision(3) << sd.variance
                    << setw(10) << setprecision(3) << sd.std_dev
                    << setw(10) << setprecision(3) << sd.sample_size
                    << "    "   << key
                    << endl;
          }
        }

        summary_file << summary.str();
      }
      
      enabled = trace && (rng->sprng() < proportion);
      
      if (!enabled && trace_file.is_open()) {
        trace_file.close();
      }
    }

    windows++;
  }

  void sampling_module::finalize() {
    if (rng) {
      //rng->free_sprng();
      rng = NULL;
    }
    if (trace_file.is_open()) {
      trace_file.close();
    }
  }


  void sampling_module::add_guide_key(const effort_key& key) {
    guide.insert(key);
  }


  Callpath make_path(const string& path) {
    vector<string> frame_strings;
    vector<FrameId> frames;

    stringutils::split(path, ":", frame_strings);
    for (size_t i=0; i < frame_strings.size(); i++) {
      char *err;
      uintptr_t offset = strtol(frame_strings[i].c_str(), &err, 0);
      frames.push_back(FrameId(ModuleId(), offset));
    }
    reverse(frames.begin(), frames.end());

    return Callpath::create(frames);
  }



}

