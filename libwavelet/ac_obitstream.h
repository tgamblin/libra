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
#ifndef AC_OBITSTREAM_H
#define AC_OBITSTREAM_H

#include <ostream>
#include <memory>
#include "buffered_obitstream.h"
#include "byte_budget_exception.h"
class Adaptive_Bit_Model;
class Arithmetic_Codec;

namespace wavelet {
  
  class ac_obitstream : public obitstream {
  protected:
    std::ostream& out;                  /// Output stream to write coded data to.
    const size_t bufsize;                 /// Buffer size for encoder
    const size_t byte_budget;             /// Max number of bytes in output.
    size_t cur_bits;                      /// Bits written out since last flush.

    std::auto_ptr<Adaptive_Bit_Model> bit_model;       /// Bit model for EZW encoding
    std::auto_ptr<Arithmetic_Codec> encoder;           /// Arithmetic coder for the stream
    size_t bits;                          /// Total number of bits written to this stream.
    size_t coded_bytes;                   /// Total number of encoded bytes written out.

  public:
    /// Constructs an arithmetic coder stream to wrap the provided output stream.
    ac_obitstream(std::ostream& out, size_t byte_budget = 0, size_t bufsize = DEFAULT_BIT_BUFSIZE);

    /// Destructor does nothing
    virtual ~ac_obitstream();

        /// Puts a zero at the end of output.
    virtual void put_zero();

    /// Puts a one at the end of output.
    virtual void put_one();

    /// Skip to beginning of next byte in output.
    virtual void next_byte();

    /// Stops encoder and writes encoded bytes to output stream.
    virtual void flush();

    /// Number of bits written to stream
    virtual size_t get_in_bits();

    /// Number of bytes written to stream
    virtual size_t get_in_bytes();

    /// Number of coded bits written out (by flushes)
    virtual size_t get_out_bits();
    
    /// Number of coded bytes written out (by flushes)
    virtual size_t get_out_bytes();

    virtual void write_bits(const unsigned char *src, size_t src_bits, size_t src_offset = 0);
  };
  
}

#endif // AC_OBITSTREAM_H
