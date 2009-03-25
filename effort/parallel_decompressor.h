#ifndef PARALLEL_DECOMPRESSOR_H
#define PARALLEL_DECOMPRESSOR_H

#include <string>
#include "wavelet.h"
#include "effort_params.h"
#include "effort_data.h"
#include "Timer.h"

namespace effort {

  /// NOTE: This isn't as scalable as parallel_compressor because the EZW decoding
  ///       is not done in parallel.  However, this is only used for validating 
  ///       compression on pre-recorded data right now, so we don't need the same kind
  ///       of performance.
  ///
  /// TODO: Create a par_ezw_decoder that does an rle_scatter and local ezw_decoding.
  ///       This will probably require changing our file format to include block sizes.
  ///       Right now the encoding used in the output file is entirely embedded.  If 
  ///       parallel_decompressor starts getting used more, we'll need scalable ways 
  ///       to do these things.
  /// 
  class parallel_decompressor {
  public:
    /// Construct a new parallel compressor
    parallel_decompressor();

    /// Distributes work to subcommunicators and delegates to do_compress() to do the
    /// actual encoding.
    void decompress(effort_data& effort_log, MPI_Comm comm_world);

    /// Sets directory to read compressed data from
    void set_input_dir(const std::string& dir) { 
      input_dir = dir;
    }
    
    size_t get_blocks() {
      return blocks;
    }

  private:
    /// Helper for distribute_work().  Actually does the work of compression on a subcommunicator
    void do_decompression(wavelet::wt_matrix& mat, effort_key key, const std::string& file, MPI_Comm comm);

    size_t blocks;    // blocks used in last decoded file.
    std::string input_dir;
  };

} //namespace


#endif //PARALLEL_DECOMPRESSOR_H 
