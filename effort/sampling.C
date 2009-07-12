#include "sampling.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <cmath>
#include <algorithm>
#include <iostream>
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
      rng(new LCG()),
      initial_sample(initial_sample_size)
  { }


  void sampling_module::init(MPI_Comm comm, double confidence, double error, std::string output_dir) {
    int rank, size;
    PMPI_Comm_rank(comm, &rank);
    PMPI_Comm_size(comm, &size);

    this->comm = comm;
    this->confidence = confidence;
    this->error = error;

    // init random number generator across procsses
    rng->init_sprng(rank, size, SEED, SPRNG_DEFAULT);

    // use first random number to figure out who's enabled to start with.
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
      summary_filename << output_dir << "/ampl_summary";
      summary_file.open(summary_filename.str().c_str());
    }
  }


  sampling_module::~sampling_module() {
    if (rng) rng->free_sprng();
  }


  void sampling_module::set_windows_per_update(size_t wpu) {
    windows_per_update = wpu;
  }


  static size_t sample_for_key(effort_data& log, effort_key& key, double confidence, double error, 
                               int root, MPI_Comm comm) {
    int rank, N;
    PMPI_Comm_rank(comm, &rank);
    PMPI_Comm_size(comm, &N);

    // don't do computation at root
    if (rank != root) return 0;

    double val  = log[key].current;
    double val2 = val * val;
    
    // compute sum and sum of squares
    double sum, sum2;
    PMPI_Reduce(&val,  &sum,  1, MPI_DOUBLE, MPI_SUM, root, comm);
    PMPI_Reduce(&val2, &sum2, 1, MPI_DOUBLE, MPI_SUM, root, comm);
    
    double mean = sum/N;                            // sample mean
    double variance = (sum2/N - (mean*mean));       // estimate w/sample mean
    double stdDev = sqrt(variance);
  
    // calculate min sample size
    double Za = computeConfidenceInterval(confidence);   // double-tailed norm conf. interval
    double d = mean * error;                             // real error bound (error var is a %)
    double V = (d/(Za*stdDev));

    size_t min_sample_size = llround(N / (1 + N * V*V));

    return min_sample_size;
  }


  double sampling_module::compute_sample_proportion(effort_data& log) {
    int rank, size;
    PMPI_Comm_rank(comm, &rank);
    PMPI_Comm_size(comm, &size);

    // sync up keys first.
    synchronize_effort_keys(log, comm);
    
    // Vector to hold keys in identical order across processes
    vector<effort_key> sorted_keys;

    // Dump keys into the vector.
    sorted_keys.reserve(log.size());
    transform(log.begin(), log.end(), back_inserter(sorted_keys), get_first());

    // Sort vector using heavy key comparison (cmpares by all frames, full module names, offsets)
    sort(sorted_keys.begin(), sorted_keys.end(), effort_key_full_lt());
    
    size_t max_sample_size = 0;
    size_t k = 0;
    while (k < sorted_keys.size()) {
      for (int root=0; root < size && k < sorted_keys.size(); root++, k++) {
        size_t cur_sample_size = sample_for_key(log, sorted_keys[k], confidence, error, root, comm);
        max_sample_size = max(cur_sample_size, max_sample_size);
      }

      size_t global_max;
      PMPI_Allreduce(&max_sample_size, &global_max, 1, MPI_SIZE_T, MPI_MAX, comm);
      max_sample_size = global_max;
    }
    
    return max_sample_size / (double)size;
  }


  void sampling_module::sample_step(effort_data& log) { 
    if (enabled) {
      // open trace file for writing if needed.
      if (!trace_file.is_open()) {
        trace_file.open(trace_filename.c_str(), ios::app);
      }

      int size;
      MPI_Comm_size(comm, &size);

      ostringstream extra;
      extra << "SampleSize " << (size_t)(proportion * size);
      extra << "Proportion " << proportion;
      
      log.write_current_step(trace_file);
    }

    proportion = compute_sample_proportion(log);
    enabled = (rng->sprng() < proportion);

    if (!enabled && trace_file.is_open()) {
      trace_file.close();
    }
  }

  void sampling_module::finalize() {
    if (rng) rng->free_sprng();
    if (trace_file.is_open()) {
      trace_file.close();
    }
  }
   
}

