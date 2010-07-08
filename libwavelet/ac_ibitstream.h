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
#ifndef IAC_STREAM_H
#define IAC_STREAM_H

#include <istream>
#include <memory>
#include "ibitstream.h"
#include "buffered_ibitstream.h"
class Adaptive_Bit_Model;
class Arithmetic_Codec;

namespace wavelet {

  class ac_ibitstream : public ibitstream {
  protected:
    std::istream& in;
    const size_t bufsize;                         /// Size of decoder buffer
    std::auto_ptr<Adaptive_Bit_Model> bit_model;  /// Bit model for EZW encoding
    std::auto_ptr<Arithmetic_Codec> decoder;      /// Arithmetic coder for the stream

    /// Max bytes to read in before throwing exception, or 0 if unlimited.
    const size_t byte_budget;

    size_t cur_bits;      /// current bits read from buffer
    size_t bits;          /// bits in buffer
    size_t total_bits;    /// total bits read in so far

    /// Reads a single encoded block from the input stream.
    void read_in();

  public:
    /// Construct a coded bitstream to read from the input stream.
    ac_ibitstream(std::istream& in, size_t byte_budget = 0, size_t bufsize = DEFAULT_BIT_BUFSIZE);

    /// Destructor
    virtual ~ac_ibitstream();

    /// Returns next bit from stream in the lowest bit of the result.
    virtual unsigned get_bit();
    
    /// True if input is still avaialble.
    virtual bool good();

    /// Gets number of bits read in from underlying stream.
    virtual size_t get_in_bytes();

    /// Skip to next aligned byte.
    virtual void next_byte();
    
  };

} // namespace

#endif // IAC_STREAM_H
