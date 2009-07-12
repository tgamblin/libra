#ifndef SAMPLING_H
#define SAMPLING_H

#include <mpi.h>

#include <string>
#include <fstream>

#include "effort_data.h"
#include "effort_key.h"
#include "RNGenerator.h"

class Sprng; /// Scalable parallel random number generator

namespace effort {

  class sampling_module {
    MPI_Comm comm;               /// communicator on which we sample.

    std::string trace_dir;       /// Name of directory to write trace data into.
    std::ofstream summary_file;  /// file stream for summary.  Valid on process 0.

    bool enabled;                /// Whether this process is enabled right now.
    double proportion;           /// Proportion to sample
    
    double confidence;           /// confidence bound for sampling
    double error;                /// error bound for sampling (percentage)
    
    size_t windows;              /// Windows seen so far.
    size_t windows_per_update;   /// Number of windows before we update sample proportion

    std::ofstream trace_file;   /// Per-process trace file.
    std::string trace_filename;  /// Name of trace file, so we can open and close.

    Sprng *rng;                  /// Uniform parallel RN generator
    
    const size_t initial_sample; /// Settable in constructor.  Defaults to 40.

    ///
    /// Collective operation.  
    /// Computes minimum sample proportion to guarantee provided confidence and error bounds.
    ///
    double compute_sample_proportion(effort_data& log);

  public:
    /// Constructor.  Doesn't init anything, but can optionally set initial sample size here.
    sampling_module(size_t initial_sample_size = 40);

    /// Destroys SPRNG if necessary
    ~sampling_module();

    /// This is a collective operation.  Inits file streams and paralle random number
    /// generator.
    void init(MPI_Comm comm, double confidence, double error, std::string output_dir);

    
    void set_windows_per_update(size_t wpu);

    /// Record end of a window.  Possibly update.
    void sample_step(effort_data& log);

    /// call to free up resources before MPI_Finalize
    void finalize();
  };

}

#endif // SAMPLING_H
