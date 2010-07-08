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
#ifndef BUFFERED_IBITSTREAM_H
#define BUFFERED_IBITSTREAM_H

#include <istream>
#include "ibitstream.h"
#include "buffered_obitstream.h"

namespace wavelet {

  // Class for reading individual bits at a time from an input stream.
  // 
  // TODO: buffer input.
  // TOOD: support reading large numbers of bits at a time.
  class buffered_ibitstream : public ibitstream {
  protected:
    std::istream& in;                /// input stream to read bits from.

    std::vector<unsigned char> buf;  /// Buffer or input characters
    size_t pos;                      /// Position in buffer to read next byte
    unsigned char mask;              /// Mask of next bit to read.
    size_t end;                      /// End of buffer in memory (1 past last byte)

    bool eof;       /// EOF flag
    
    /// Buffers data from underlying stream for reading and resets pointer
    virtual void reload();
    
  public:
    /// Constructs an buffered_ibitstream to read from the prvided input stream.
    /// TODO: support byte budget.
    buffered_ibitstream(std::istream& in, size_t byte_budget = 0, size_t bufsize = DEFAULT_BIT_BUFSIZE);

    /// Destructor; does nothing.
    virtual ~buffered_ibitstream();

    /// Gets a single bit of input.  Left in header to enable inlining.
    unsigned get_bit() {
      if (pos == end) {
	reload();
      }

      unsigned bit = (mask & buf[pos]) ? 1 : 0;
      mask >>= 1;

      if (mask == 0) {
	pos++;
	mask = 0x80;
      }

      return bit;
    }

    /// True if there are more bits to be input.
    virtual bool good() {
      return !(pos == end && eof);
    }

    virtual size_t get_in_bytes() {
      return 0;
    }

    virtual void next_byte();

  }; // buffered_ibitstream

} // namespace

#endif // BUFFERED_IBITSTREAM_H

