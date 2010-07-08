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
#include "ac_ibitstream.h"

#include "arithmetic_codec.h"
#include "byte_budget_exception.h"
#include "wt_utils.h"
#include "io_utils.h"

using namespace std;

namespace wavelet {

  ac_ibitstream::ac_ibitstream(istream& i, size_t bb, size_t bs) 
    : in(i), bufsize(bs), 
      bit_model(new Adaptive_Bit_Model()), decoder(new Arithmetic_Codec(bs)), 
      byte_budget(bb), cur_bits(0), 
      bits(0), total_bits(0)
  {
    decoder->start_decoder();
  }
  

  ac_ibitstream::~ac_ibitstream() { 
    
  }


  void ac_ibitstream::read_in() {
    decoder->stop_decoder();

    size_t bytes = vl_read(in);
    size_t enc_bytes = vl_read(in);
    bits = bytes << 3;
    
    if (bytes > bufsize) {
      cerr << "Error: decoder bufsize too small." << endl;
      exit(1);
    }

    in.read((char*)decoder->buffer(), enc_bytes);

    cur_bits = 0;
    decoder->start_decoder();
  }

  
  unsigned ac_ibitstream::get_bit() {
    if (byte_budget && (total_bits >= (byte_budget << 3))) {
      throw byte_budget_exception();
    }
    
    if (cur_bits == bits) {
      read_in();
    }

    cur_bits++;
    total_bits++;
    return decoder->decode(*bit_model);
  }
  
  
  bool ac_ibitstream::good() {
    return (cur_bits < bits || (in.peek() != EOF && in.good()));
  }

  size_t ac_ibitstream::get_in_bytes() {
    return bits_to_bytes(total_bits);
  }


  void ac_ibitstream::next_byte() {
    while (total_bits & 0x7l) get_bit();
  }
  
} // namespace
