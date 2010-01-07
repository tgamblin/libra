#ifndef SAMPLING_H
#define SAMPLING_H

#include <mpi.h>

#include <string>
#include <fstream>
#include <set>

#include "effort_data.h"
#include "effort_key.h"
#include "Callpath.h"
#include "Timer.h"
#include "string_utils.h"


class Sprng; /// Scalable parallel random number generator

namespace effort {

  struct sample_desc {
    double mean;
    double variance;
    double std_dev;
    size_t sample_size;
    
    sample_desc() : mean(0), variance(0), std_dev(0), sample_size(0) { }
    sample_desc(double m, double v, double sd, size_t ss)
      : mean(m), variance(v), std_dev(sd), sample_size(ss) { }
  };

  typedef std::map<effort_key, sample_desc> stat_map;
  

  class Sampler {
    MPI_Comm comm;               /// communicator on which we sample.

    std::string trace_dir;       /// Name of directory to write trace data into.
    std::ofstream summary_file;  /// file stream for summary.  Valid on process 0.

    bool enabled;                /// Whether this process is enabled right now.
    double proportion;           /// Proportion to sample
    
    double confidence;           /// confidence bound for sampling
    double error;                /// error bound for sampling (percentage)
    bool normalized_error;       /// whether error is normalized or absolute bound
    
    size_t windows;              /// Windows seen so far.
    size_t windows_per_update;   /// Number of windows before we update sample proportion
    bool record_stats;           /// record summary stats or not
    bool trace;                  /// write trace files or not
    size_t max_strata;           /// Max number of strata to produce
    int sig_level;               /// Transform level for signatures used for stratifying.
    
    std::ofstream trace_file;    /// Per-process trace file.
    std::string trace_filename;  /// Name of trace file, so we can open and close.

    Sprng *rng;                  /// Uniform parallel RN generator
    
    size_t initial_sample;       /// Settable in constructor.  Defaults to 40.
    std::set<effort_key> guide;  /// Effort keys for regions that guide sapmling

    Timer timer;                 /// Performance timer.

    ///
    /// Compute minimum proportion of processes to sample on communicator comm 
    /// to guarantee confidence and error bounds.  This can compute sample size for
    /// multiple metrics (identified by effort keys).  It will return the *maximum* 
    /// sample size for any effort key evaluated.
    ///
    /// NOTE: This is a collective operation.
    /// 
    /// PRE: log contains records for all keys in the keys vector.
    /// PRE: keys vector is identical (and in same order) on all processes.
    ///
    /// Parameters:
    ///   log     effort_data with collected data from all processes.
    ///   keys    keys from log to evaluate sample size for.
    ///   stats   If record_stats is true, summary statistics for all keys will be stored here on process 0.
    ///   comm    Communicator whose processes we'll evaluate.
    ///
    double compute_sample_proportion(
      effort_data& log, const std::vector<effort_key>& keys, stat_map& stats, MPI_Comm comm) const;

    ///
    /// Computes the minimum sample size for a population 
    ///
    static sample_desc sample_size(
      double sum, double sum2, size_t N, double confidence, double error, bool normalize);

    void get_sample_keys(effort_data& log, std::vector<effort_key>& sorted_keys);

  public:
    /// Constructor.  Doesn't init anything, but can optionally set initial sample size here.
    Sampler(size_t initial_sample_size = 40);

    /// Destroys SPRNG if necessary
    ~Sampler();

    /// This is a collective operation.  Inits file streams and paralle random number
    /// generator.
    void init(MPI_Comm comm, double confidence, double error, std::string output_dir);

    
    void set_windows_per_update(size_t wpu);
    void set_normalized_error(bool normalized);
    void set_stats(bool stats);
    void set_trace(bool trace);
    void add_guide_key(const effort_key& key);
    void set_max_strata(size_t max);
    void set_sig_level(int level);

    /// Record end of a window.  Possibly update.
    void sample_step(effort_data& log);

    /// call to free up resources before MPI_Finalize
    void finalize();
    
    /// Get timer for sampling events.
    const Timer& get_timer() { return timer; }
  };

}

#endif // SAMPLING_H
