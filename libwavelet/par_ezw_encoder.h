/////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010, Lawrence Livermore National Security, LLC.  
// Produced at the Lawrence Livermore National Laboratory  
// Written by Todd Gamblin, tgamblin@llnl.gov.
// LLNL-CODE-417602
// All rights reserved.  
// 
// This file is part of Libra. For details, see http://github.com/tgamblin/libra.
// Please also read the LICENSE file for further information.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the disclaimer below.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the disclaimer (as noted below) in the documentation and/or other materials
//    provided with the distribution.
//  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
//    or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
/////////////////////////////////////////////////////////////////////////////////////////////////
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
    size_t encode(wt_matrix& mat, std::ostream& out, int level = -1, 
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
