#include "sampler.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

#include "stl_utils.h"
#include "synchronize_keys.h"
#include "effort_key.h"
#include "ltqnorm.h"

#include "par_kmedoids.h"
#include "effort_signature.h"

#define USE_MPI
#include "sprng_cpp.h"

#define SEED 985456376

using namespace cluster;


namespace effort {

  Sampler::Sampler(size_t initial_sample_size)
    : comm(MPI_COMM_NULL), 
      enabled(false), 
      proportion(0), 
      confidence(0), 
      error(0),
      windows(0),
      windows_per_update(5),
      record_stats(false),
      trace(true),
      max_strata(1),
      rng(new LCG()),
      initial_sample(initial_sample_size)
  { }


  void Sampler::init(MPI_Comm comm, double confidence, double error, std::string output_dir) {
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


  Sampler::~Sampler() {
    if (rng) {
      rng->free_sprng(); 
      rng = NULL;
    }
  }


  void Sampler::set_windows_per_update(size_t wpu) {
    windows_per_update = wpu;
  }


  void Sampler::set_normalized_error(bool normalized) {
    normalized_error = normalized;
  }


  void Sampler::set_stats(bool stats) {
    record_stats = stats;
  }


  void Sampler::set_trace(bool trace) {
    this->trace = trace;
  }

  void Sampler::set_strata(size_t s) {
    max_strata = s;
  }

  void Sampler::set_sig_level(int level) {
    sig_level = level;
  }


  sample_desc Sampler::sample_size(
    double sum, double sum2, size_t N, double confidence, double error, bool normalize) 
  {
    double mean = sum/N;                            // sample mean
    double variance = (sum2/N - (mean*mean));       // estimate w/sample mean
    double stdDev = sqrt(variance);

    if (stdDev < 1e-9) stdDev = 1e-9;                // in case variance is 0.

    // calculate min sample size
    double Za = computeConfidenceInterval(confidence);   // double-tailed norm conf. interval
    double d = error;
    if (normalize) d *= mean;           // if normalized, error is a %, so multiply by mean.

    double V = (d/(Za*stdDev));

    size_t min_sample_size = llround(N / (1 + N * V*V));

    return sample_desc(mean, variance, stdDev, min_sample_size);
  }

  struct guide_check {
    set<effort_key>& guide;
    guide_check(set<effort_key>& g) : guide(g) { }

    bool operator()(const effort_key& key) {
      if (guide.empty()) return true;   // empty set => all true.

      for (set<effort_key>::iterator i=guide.begin(); i != guide.end(); i++) {
        if (key.start_path.in(i->start_path) && key.end_path.in(i->end_path)) {
          return true;
        }
      }
      return false;

      // TODO: figure out what to do about key matching.
      //effort_key normalized(Metric::time(), 0, key.start_path, key.end_path);
      //return (guide.find(normalized) != guide.end());
    }
  };


  void Sampler::get_sample_keys(effort_data& log, vector<effort_key>& keys) {
    keys.clear();

    // Dump keys into the vector.
    keys.reserve(log.size());
    transform(log.begin(), log.end(), back_inserter(keys), get_first());

    // remove non-guiding keys.
    keys.erase(std::partition(keys.begin(), keys.end(), guide_check(guide)), keys.end());

    // Sort vector using heavy key comparison (cmpares by all frames, full module names, offsets)
    sort(keys.begin(), keys.end(), effort_key_full_lt());

    // if we're stratifying, we only want the first key.
    // TODO: figure out how to deal with multiple keys and stratification.
    if (max_strata > 1) {
      if (keys.size() < 1) {
        cerr << "ERROR: can't stratify with no keys!" << endl;
        exit(1);
      }
      keys.resize(1);
    }
  }


  struct variance_gt {
    const stat_map& stats;
    variance_gt(const stat_map& sm) : stats(sm) { }
    bool operator()(const effort_key& lhs, const effort_key& rhs) {
      return stats.find(lhs)->second.variance > stats.find(rhs)->second.variance;
    }
  };

  struct sample_size_gt {
    const stat_map& stats;
    sample_size_gt(const stat_map& sm) : stats(sm) { }
    bool operator()(const effort_key& lhs, const effort_key& rhs) {
      return stats.find(lhs)->second.sample_size > stats.find(rhs)->second.sample_size;
    }
  };

  ostream& operator<<(ostream& out, stat_map& stats) {
    vector<effort_key> ss_ordered;
    transform(stats.begin(), stats.end(), back_inserter(ss_ordered), get_first());
    sort(ss_ordered.begin(), ss_ordered.end(), sample_size_gt(stats));
    
    for (size_t i=0; i < ss_ordered.size(); i++) {
      const effort_key& key = ss_ordered[i];
      const sample_desc& sd = stats[key]; 
     
      out << setw(4)  << i
          << setw(10) << setprecision(3) << sd.mean
          << setw(10) << setprecision(3) << sd.variance
          << setw(10) << setprecision(3) << sd.std_dev
          << setw(10) << setprecision(3) << sd.sample_size
          << "    "   << key
          << endl;
    }
    return out;
  }
  

  double Sampler::compute_sample_proportion(effort_data& log, 
                                            const vector<effort_key>& keys,
                                            stat_map& stats,
                                            MPI_Comm comm) const
  {
    int rank, size;
    PMPI_Comm_rank(comm, &rank);
    PMPI_Comm_size(comm, &size);

    size_t local_max_sample_size = 0;
    size_t k = 0;
    while (k < keys.size()) {
      // reduce per-key sum and sum squares to different processes
      double sum, sum2;

      int root;
      for (root=0; root < size && k < keys.size(); root++, k++) {
        effort_key key = keys[k];
        double val  = log[key].current;
        double val2 = val * val;

        // compute sum and sum of squares
        PMPI_Reduce(&val,  &sum,  1, MPI_DOUBLE, MPI_SUM, root, comm);
        PMPI_Reduce(&val2, &sum2, 1, MPI_DOUBLE, MPI_SUM, root, comm);
      }

      sample_desc sd;
      if (rank < root) {
        // calculate local sample size and take the max of sizes seen so far.
        sd = sample_size(sum, sum2, size, confidence, error, normalized_error);
        local_max_sample_size = max(sd.sample_size, local_max_sample_size);
      }

      // gather sample_descs to proc 0
      if (record_stats) {
        MPI_Comm gather_comm;
        PMPI_Comm_split(comm, (rank < root ? 0 : 1), rank, &gather_comm);

        vector<sample_desc> vars(root);
        if (rank < root) {
          PMPI_Gather(&sd,      sizeof(sample_desc), MPI_BYTE,
                      &vars[0], sizeof(sample_desc), MPI_BYTE,
                      0, gather_comm);

          if (rank == 0) {

            for (int i=0; i < root; i++) {
              stats[keys[k - root + i]] = vars[i];
            } 
          }
        }

        PMPI_Comm_free(&gather_comm);
      }
    }

    // find global sample size for this group with allreduce.
    size_t max_sample_size;
    PMPI_Allreduce(&local_max_sample_size, &max_sample_size, 1, MPI_SIZE_T, MPI_MAX, comm);

    // in case there's really no variance.
    if (max_sample_size < 1) max_sample_size = 1;

    return max_sample_size / (double)size;
  }






  void Sampler::sample_step(effort_data& log) { 
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

    timer.fast_forward();  // start timing now.   Timing doesn't include logging.
    
    if (windows % windows_per_update == 0) {
      // sync up effort keys and extract only those we're sampling.
      vector<effort_key> keys;
      synchronize_effort_keys(log, comm);
      get_sample_keys(log, keys);
      timer.record("SyncKeys");
      
      // =========================== Stratification =========================== //
      par_kmedoids km(comm);
      MPI_Comm stratum_comm = comm;
      size_t num_strata = 1;

      if (max_strata > 1 && log.steps() > 0) {  // only start stratifying on 2nd update.
        effort_key my_key        = log.begin()->first;
        effort_record& my_record = log.begin()->second;

        vector<effort_signature> my_trace;
        double *start = &my_record[my_record.size() - windows_per_update];
        my_trace.push_back(effort_signature(start, windows_per_update, sig_level));
        timer.record("MakeSignature");

        km.xclara(my_trace, sig_euclidean_distance(), max_strata, windows_per_update);
        timer += km.get_timer();
        timer.fast_forward();

        int my_id = km.cluster_ids[0];  // cluster id tells us what stratum we're in
        PMPI_Comm_split(comm, my_id, rank, &stratum_comm);
        timer.record("CommSplit");
        
        num_strata = km.medoid_ids.size();
      }

      // ====================================================================== //
      
      stat_map local_stats;
      proportion = compute_sample_proportion(log, keys, local_stats, stratum_comm);
      timer.record("SampleProportion");
      
      // init these with values that work if there's one stratum.
      double proportions[num_strata];
      size_t sizes[num_strata];
      cluster::partition strata;      
      vector<stat_map> all_stats;
      
      if (max_strata > 1 && log.steps() > 0) {
        double local_proportions[num_strata];
        size_t local_sizes[num_strata];
        
        for (size_t i=0; i < num_strata; i++) {
          local_proportions[i] = 0.0;
          local_sizes[i] = 0;
        }
        
        // rank 0 from each stratum records its stats; everyone else is zero
        int srank, ssize;
        PMPI_Comm_rank(stratum_comm, &srank);
        PMPI_Comm_size(stratum_comm, &ssize);
        
        medoid_id my_cluster = km.cluster_ids[0];
        if (srank == 0) {
          local_proportions[my_cluster] = proportion;
          local_sizes[my_cluster]       = ssize;
        }

        PMPI_Reduce(local_proportions, proportions, num_strata, MPI_DOUBLE, MPI_SUM, 0, comm);
        PMPI_Reduce(local_sizes, sizes, num_strata, MPI_SIZE_T, MPI_SUM, 0, comm);
        timer.record("SampleProportion");

        if (record_stats) {
          // hihger-overhead stuff, like gathering ids and keys, happens in here.
          // only produce this if we need more precise data.
          km.gather(strata, 0);
        
          for (size_t s=0; s < strata.medoid_ids.size(); s++) {
            if (srank == 0 && rank == 0) {
              // just push local stats on the stas vector.
              all_stats.push_back(local_stats);
              
            } else {
              vector<effort_key> keys;
              get_sample_keys(log, keys);
              vector<sample_desc> summaries(keys.size());
              
              if (srank == 0) {
                for (size_t i=0; i < keys.size(); i++) {
                  summaries[i] = local_stats[keys[i]];
                }
                
                PMPI_Send(&summaries[0], keys.size() * sizeof(sample_desc), MPI_BYTE, 
                          0, 0, comm);
                
              } else if (rank == 0) {
                PMPI_Recv(&summaries[0], keys.size() * sizeof(sample_desc), MPI_BYTE, 
                          (int)km.medoid_ids[s], 0, comm, MPI_STATUS_IGNORE);
                
                all_stats.push_back(stat_map());
                for (size_t i=0; i < keys.size(); i++) {
                  all_stats.back()[keys[i]] = summaries[i];
                }
              }
            }
          }
          timer.record("Stats");
        }

      } else if (rank == 0) {
        // single cluster.
        all_stats.push_back(local_stats);
        proportions[0] = proportion;
        sizes[0] = size;
      }


      if (rank == 0) {
        ostringstream summary;
        summary <<         "STEP " << log.progress_count << endl;
        if (max_strata > 1) {
          summary <<       "    Strata  " << num_strata << endl;
        }

        for (size_t i=0; i < num_strata; i++) {
          if (max_strata > 1) {
            summary <<     "    Stratum " << i << endl;
            summary <<     "        Size     " << sizes[i] << endl;

            if (record_stats) {
              if (log.steps() == 0) {
                summary << "        Members [0-" << (size-1) << "]" << endl;;
              } else {
                summary << "        Members [" << strata.members(i) << "]" << endl;;
              }
            }
          }

          summary <<       "        SampleSize " << (size_t)(proportions[i] * sizes[i]) << endl;
          summary <<       "        Proportion " << proportions[i] << endl;

          if (record_stats) {
            summary <<     "        Keys       " << all_stats[i].size() << endl;
            summary << all_stats[i];
          }
        }
        summary_file << summary.str();
        timer.record("WriteSummary");
      }
      
      enabled = trace && (rng->sprng() < proportion);
      
      if (!enabled && trace_file.is_open()) {
        trace_file.close();
      }
    }
    
    windows++;
  }
  

  void Sampler::finalize() {
    timer.fast_forward();
    if (rng) {
      //rng->free_sprng();
      rng = NULL;
    }
    if (trace_file.is_open()) {
      trace_file.close();
    }
    timer.record("SamplerFinalize");
  }


  void Sampler::add_guide_key(const effort_key& key) {
    guide.insert(key);
  }

}

