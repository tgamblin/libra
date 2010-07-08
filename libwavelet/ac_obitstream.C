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
#include "ac_obitstream.h"
#include "wt_utils.h"
#include "io_utils.h"
using namespace std;

#include "arithmetic_codec.h"
#include "byte_budget_exception.h"

#define CHECK_BYTES \
do { \
    if (byte_budget && (encoder->get_code_bytes() + get_out_bytes()) >= byte_budget) { \
      throw byte_budget_exception(); \
    } \
} while (0) 


namespace wavelet {

  ac_obitstream::ac_obitstream(ostream& o, size_t bb, size_t bs) 
    : out(o), bufsize(bs), byte_budget(bb), cur_bits(0), 
      bit_model(new Adaptive_Bit_Model()), encoder(new Arithmetic_Codec(2*bs)), 
      bits(0), coded_bytes(0)
  { 
    encoder->start_encoder();
  }
  
  
  ac_obitstream::~ac_obitstream() { 
    flush();
  }
  
  
  void ac_obitstream::put_zero() {
    CHECK_BYTES;

    encoder->encode((unsigned)0, *bit_model);
    cur_bits++;
    bits++;
    if ((cur_bits >> 3) == bufsize) {
      flush();
    }
  }


  void ac_obitstream::put_one() {
    CHECK_BYTES;

    encoder->encode((unsigned)1, *bit_model);
    cur_bits++;
    bits++;
    if ((cur_bits >> 3) == bufsize) {
      flush();
    }
  }
  

  void ac_obitstream::next_byte() {
    while (bits & 0x7l) {
      put_zero();
    }
  }

  
  void ac_obitstream::flush() {
    if (cur_bits) {
      long cur_bytes = bits_to_bytes(cur_bits);
      long enc_bytes = encoder->stop_encoder();

      coded_bytes += vl_write(out, cur_bytes);
      coded_bytes += vl_write(out, enc_bytes);
      out.write((char*)encoder->buffer(), enc_bytes);
      coded_bytes += enc_bytes;

      cur_bits = 0;
      encoder->start_encoder();
    }
  }


  size_t ac_obitstream::get_in_bits() {
    return bits;
  }
  

  size_t ac_obitstream::get_in_bytes() {
    return bits_to_bytes(bits);
  }
  

  size_t ac_obitstream::get_out_bits() {
    return (coded_bytes << 3);
  }


  size_t ac_obitstream::get_out_bytes() {
    return coded_bytes;
  }

  void ac_obitstream::write_bits(const unsigned char *src, size_t src_bits, size_t src_offset) {
    size_t pos = (src_offset >> 3);
    unsigned char mask = (0x80 >> (src_offset & 0x7l));

    for (size_t i=0; i < src_bits; i++) {
      unsigned bit = (src[pos] & mask) ? 1 : 0;
      encoder->encode(bit, *bit_model);
      cur_bits++;
      bits++;
      if ((cur_bits >> 3) == bufsize) {
	flush();
      }

      mask >>= 1;

      if (mask == 0) {
	pos++;
	mask = 0x80;
      }
    }
  }


} //namespace
