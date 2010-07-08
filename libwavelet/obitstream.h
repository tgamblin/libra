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
#ifndef OBITSTREAM_H
#define OBITSTREAM_H

#include <cstdlib>
#define DEFAULT_BIT_BUFSIZE 65536

namespace wavelet {

  /// Interface class for output bit streams.  Contains default implementations of
  /// some convenience methods.
  class obitstream {
  public:
    /// Cnstructor
    obitstream();

    /// Destructor
    virtual ~obitstream();

    /// Puts a zero at the end of output.
    virtual void put_zero() = 0;

    /// Puts a one at the end of output.
    virtual void put_one() = 0;

    /// Gets number of bits written to this obitstream.
    virtual size_t get_in_bits() = 0;
    
    /// Gets number of bytes written to this obitstream.
    virtual size_t get_in_bytes() = 0;
    
    /// Gets number of bits written out by this obitstream.
    virtual size_t get_out_bits() = 0;
    
    /// Gets number of bytes written out by this obitstream.
    virtual size_t get_out_bytes() = 0;
    
    /// Flushes output to underlying output stream.
    virtual void flush() = 0;

    /// Skips to beginning of next byte, filling with zeros.  If we're already about
    /// to write to the beginning of the next byte, just stay there.
    virtual void next_byte() = 0;
    
    /// Writes raw bytes to the bit stream.  Default implementation just calls 
    /// write_bits with offset zero.  Note that this can lose bit alignment
    virtual void write(const unsigned char *buf, size_t bytes);
    
    /// Writes out all bits in a buffer.  Optionally,  an offset into the buffer 
    /// can be specified as a starting point for reading.  Default implementation
    /// calls put_zero and put_one.
    virtual void write_bits(const unsigned char *buf, size_t nbits, size_t offset = 0);

    /// Convenience method -- default impleemntation just calls put_zero or put_one.
    virtual void put_bit(bool bit);

  }; // obitstream

} // namespace

#endif //OBITSTREAM_H
