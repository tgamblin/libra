#ifndef SAMPLING_H
#define SAMPLING_H

#include <mpi.h>

#include <string>
#include <fstream>
#include <set>
#include <vector>

#include "effort_data.h"
#include "effort_key.h"
#include "Callpath.h"
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
    bool stats;                  /// record summary stats or not
    bool trace;                  /// write trace files or not

    std::ofstream trace_file;    /// Per-process trace file.
    std::string trace_filename;  /// Name of trace file, so we can open and close.

    Sprng *rng;                  /// Uniform parallel RN generator
    
    size_t initial_sample;       /// Settable in constructor.  Defaults to 40.
    stat_map key_stats;          /// Sample stats for each effort region.
    std::set<effort_key> guide;  /// Effort keys for regions that guide sapmling

    ///
    /// Collective operation.  
    /// Computes minimum sample proportion to guarantee provided confidence and error bounds.
    ///
    double compute_sample_proportion(effort_data& log);

    sample_desc compute_sample_size(double sum, double sum2, size_t N, double confidence, double error);

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

    /// Record end of a window.  Possibly update.
    void sample_step(effort_data& log);

    /// call to free up resources before MPI_Finalize
    void finalize();
  };

  Callpath make_path(const std::string& path);


  template <class OutputIterator>
  void parse_effort_keys(const char *str, OutputIterator output) {
    if (!str) return;

    std::vector<std::string> key_strings;
    stringutils::split(str, ", ", key_strings);
    
    for (size_t k=0; k < key_strings.size(); k++) {
      std::vector<std::string> path_strings;
      stringutils::split_str(key_strings[k], ">", path_strings);

      Callpath start(make_path(path_strings[0]));
      Callpath end(start);
      if (path_strings.size() > 1) {
        end = make_path(path_strings[1]);        
      }
      
      *output++ = effort_key(Metric::time(), 0, start, end);
    }
  }


}

#endif // SAMPLING_H
