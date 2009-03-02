#ifndef PAR_EZW_ENCODER_H
#define PAR_EZW_ENCODER_H

#include <mpi.h>
#include "ezw_encoder.h"
#include "Timer.h"

namespace wavelet {

  class par_ezw_encoder : public ezw_encoder {
  public:
    
    par_ezw_encoder();

    virtual ~par_ezw_encoder();
    
    /// Parallel version of encode().  Assumes data is distributed evenly
    /// across all processors. This reduces distributed data to a single process
    /// and writes it out ot an obitstream.  The root of the reduction is the
    /// process with rank size/2, where size is the size of comm as given by
    /// MPI_Comm_size.
    ///
    /// Useful note for using streams: only the stream on process size/2 will
    /// be written to.  So if using a file stream, ONLY open the stream on 
    /// process size/2.
    virtual size_t encode(wt_matrix& mat, std::ostream& out, int level = -1, 
			  MPI_Comm comm = MPI_COMM_WORLD);
    

    /// Sets whether this uses a traversal ordering that's compatible with the 
    /// sequential encoder.  Defaults to false.
    void set_use_sequential_order(bool use);
    
    /// Get whether this is using reduction or gather.
    bool get_use_sequential_order();

    /// Gets the root of the reduction that this will do.  May not be zero.
    int get_root(MPI_Comm comm = MPI_COMM_WORLD);

    const Timer& get_timer() { return timer; }

  protected:
    /// Whether we output EZW bits in same order as sequential coder.  Defaults to false.
    bool use_sequential_order;

    size_t bit_stitch_encode(const unsigned char *passes, size_t total_bytes, std::ostream& out, 
			     ezw_header& header, MPI_Comm comm);
    
    size_t block_encode(const unsigned char *passes, size_t total_bytes, std::ostream& out, 
			ezw_header& header, MPI_Comm comm);

    Timer timer;
  };

}

#endif // PAR_EZW_ENCODER_H
