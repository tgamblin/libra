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
#include "vector_obitstream.h"
#include "wt_utils.h"
#include <iostream>
using namespace std;

namespace wavelet {

  vector_obitstream::vector_obitstream(size_t bufsize) 
    : buf(*new vector<unsigned char>(bufsize)), my_vector(true), pos(0), mask(0x80), bits(0)
  { 
    buf[0] = 0;
  }


  vector_obitstream::vector_obitstream(vector<unsigned char>& buffer) 
    : buf(buffer), my_vector(false), pos(0), mask(0x80), bits(0)
  { 
    buf[0] = 0;
  }


  vector_obitstream::~vector_obitstream() {
    flush();
    if (my_vector) delete &buf;
  }
  
  
  void vector_obitstream::put_zero() {
    mask >>= 1;
    bits++;
    if (mask == 0) {
      pos++;
      mask = 0x80;
      
      if (pos == buf.size()) {
	flush();
      } else {
	buf[pos] = 0;
      }
    }
  }


  void vector_obitstream::put_one() {
    buf[pos] |= mask;
    put_zero();
  }
  

  void vector_obitstream::write_bits(const unsigned char *src, size_t src_bits, size_t src_offset) {
    size_t new_total = bits_to_bytes(bits + src_bits);
    if (new_total > buf.size()) {
      buf.resize(new_total * 2, 0);
    }
    insert_bits(&buf[0], src, src_bits, bits, src_offset);    
    bits += src_bits;
    pos = (bits >> 3);
    mask = (0x80 >> (bits & 0x7l));
  }


  void vector_obitstream::flush() {
    if (pos == buf.size()) {
      buf.resize(2 * buf.size(), 0);
    }
  }


  size_t vector_obitstream::get_in_bits() {
    return bits;
  }


  size_t vector_obitstream::get_in_bytes() {
    return bits_to_bytes(bits);
  }


  size_t vector_obitstream::get_out_bits() {
    return bits;
  }


  size_t vector_obitstream::get_out_bytes() {
    return bits_to_bytes(bits);
  }


  void vector_obitstream::next_byte() {
    while (mask != 0x80) {
      put_zero();
    }
  }


  /// returns pointer to internal buffer
  unsigned char *vector_obitstream::get_buffer() {
    return &buf[0];
  }


  /// returns pointer to internal buffer
  vector<unsigned char>& vector_obitstream::get_vector() {
    return buf;
  }


  void vector_obitstream::resize(size_t size) {
    buf.resize(size);
  }


  void vector_obitstream::swap(std::vector<unsigned char>& other) {
    other.swap(buf);
    pos = 0;
    mask = 0x80;
    bits = 0;
    buf[0] = 0;
  }

} // namespace 
