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
    
    // key -> filename mapping for last decompression done.
    const std::map<effort_key, std::string> *file_map() {
      return &file_for_key;
    }

  private:
    /// Helper for distribute_work().  Actually does the work of compression on a subcommunicator
    void do_decompression(wavelet::wt_matrix& mat, effort_key key, const std::string& file, MPI_Comm comm);

    size_t blocks;    // blocks used in last decoded file.
    std::string input_dir;
    std::map<effort_key, std::string> file_for_key;
  };

} //namespace


#endif //PARALLEL_DECOMPRESSOR_H 
