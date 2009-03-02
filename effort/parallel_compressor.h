#ifndef PARALELL_COMPRESSOR_H
#define PARALELL_COMPRESSOR_H

#include <string>
#include "wavelet.h"
#include "effort_params.h"
#include "effort_data.h"
#include "Timer.h"

namespace effort {

  class parallel_compressor {
  public:
    /// Construct a new parallel compressor
    parallel_compressor(const effort_params& params);

    /// Distributes work to subcommunicators and delegates to do_compress() to do the
    /// actual encoding.
    void compress(effort_data& effort_log, MPI_Comm comm_world);

    /// Sets directory to output compressed data to
    void set_output_dir(const std::string& dir) { 
      output_dir = dir;
    }

    /// Sets directory to output exact data to, if params.verify is true.
    void set_exact_dir(const std::string& dir) {
      exact_dir = dir;
    }

    /// Returns timings for last call to compress()
    const Timer& get_timer() {
      return timer;
    }

  private:
    /// Helper for distribute_work().  Actually does the work of compression on a subcommunicator
    void do_compression(wavelet::wt_matrix& mat, effort_key key, int id, MPI_Comm comm);

    const effort_params& params;
    std::string output_dir;
    std::string exact_dir;

    Timer timer;   // keeps stats on timings of compression phases
  };

} //namespace


#endif //PARALLEL_COMPRESSOR_H 
