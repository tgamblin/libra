#ifndef PARALLEL_COMPRESSOR_H
#define PARALLEL_COMPRESSOR_H

#include <string>
#include <map>
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

    void set_file_map(const std::map<effort_key, std::string> *fm) {
      file_map = fm;
    }

    /*    
    /// sets confidence interval for sample
    /// confidence should be in (0, 1]
    void set_confidence(double confidence);
    
    /// sets confidence interval for sample
    /// error should be in [0. 1)
    void set_error(double error);
    */

    MPI_Comm reorder_ranks_in_bins(effort_record& record, MPI_Comm comm);
    

  private:
    /// Helper for distribute_work().  Actually does the work of compression on a subcommunicator
    void do_compression(wavelet::wt_matrix& mat, effort_key key, int id, MPI_Comm comm);
    





    const effort_params& params;
    std::string output_dir;
    std::string exact_dir;
    const std::map<effort_key, std::string> *file_map;
    Timer timer;   // keeps stats on timings of compression phases

    bool sample_topology;

    //    double error;
    //    double confidence;
  };

} //namespace


#endif //PARALLEL_COMPRESSOR_H 
