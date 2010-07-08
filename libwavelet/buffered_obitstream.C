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
#include "buffered_obitstream.h"
#include "wt_utils.h"

using namespace std;

namespace wavelet {

  buffered_obitstream::buffered_obitstream(ostream& o, size_t byte_budget, size_t bufsize) 
    : buf(bufsize), pos(0), mask(0x80), bits(0), out(o)
  { 
    buf[0] = 0;
  }


  buffered_obitstream::~buffered_obitstream() {
    flush();
  }
  
  
  void buffered_obitstream::put_zero() {
    mask >>= 1;
    bits++;
    if (mask == 0) {
      pos++;
      mask = 0x80;
      
      if (pos == buf.size()) {
	my_flush();
      } else {
	buf[pos] = 0;
      }
    }
  }


  void buffered_obitstream::put_one() {
    buf[pos] |= mask;
    put_zero();
  }
  

  void buffered_obitstream::write_bits(const unsigned char *src, size_t src_bits, size_t src_offset) {
    size_t offset = (pos << 3);
    for (unsigned m = mask; m != 0x80; m <<= 1) offset++;
    size_t max_bits = (buf.size() << 3);

    while (src_bits) {
      size_t to_write = min(max_bits - offset, src_bits);
      insert_bits(&buf[0], src, to_write, offset, src_offset);

      src_bits -= to_write;
      src_offset += to_write;
      bits += to_write;
      offset += to_write;

      pos = (offset >> 3);                      // reset buffer index
      mask = (0x80 >> (offset & 0x7l));         // reset bit mask for last bit.
      buf[pos] &= ~(0xFF >> (offset & 0x7l));   // zero out remaining bits in trailing byte.

      if (offset == max_bits) {
	my_flush();
	offset = 0;
      }
    }
  }

  
  void buffered_obitstream::my_flush() {
    size_t buf_bits = (pos << 3) + (mask == 0x80 ? 0 : 1);
    if (buf_bits) {
      out.write(reinterpret_cast<char*>(&buf[0]), bits_to_bytes(buf_bits));
      pos = 0;
      buf[pos] = 0;
      mask = 0x80;
    }
  }


  void buffered_obitstream::flush() {
    my_flush();
    out.flush();
  }


  size_t buffered_obitstream::get_in_bits() {
    return bits;
  }


  size_t buffered_obitstream::get_in_bytes() {
    return bits_to_bytes(bits);
  }


  size_t buffered_obitstream::get_out_bits() {
    return bits;
  }


  size_t buffered_obitstream::get_out_bytes() {
    return bits_to_bytes(bits);
  }


  void buffered_obitstream::next_byte() {
    while (mask != 0x80) {
      put_zero();
    }
  }

} // namespace 
