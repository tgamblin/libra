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
#include "ezw.h"

#include <stdexcept>
#include <cmath>
#include <cstring>
#include <ostream>
using namespace std;

#include "wt_utils.h"
#include "io_utils.h"

namespace wavelet {

  ostream& operator<<(ostream& out, const ezw_header& header) {
    out << "Header: {rows: " << header.rows 
        << ", cols: "        << header.cols 
        << ", level: "       << header.level 
        << ", scale: "       << header.scale
        << ", mean: "        << header.mean
        << ", threshold: "   << header.threshold
        << ", encoding: "    << header.enc_type
        << ", blocks: "      << header.blocks
        << ", ezw_size: "    << header.ezw_size
        << ", rle_size: "    << header.rle_size
        << ", enc_size: "    << header.enc_size
        << "}";
    return out;
  }


  /// Helpful for input
  encoding_t str_to_encoding(const char *str) {
    if (strcasecmp("ARITHMETIC", str) == 0) {
      return ARITHMETIC;
    } else if (strcasecmp("HUFFMAN", str) == 0) {
      return HUFFMAN;
    } else if (strcasecmp("RLE", str) == 0) {
      return RLE;
    } else if (strcasecmp("NONE", str) == 0) {
      return NONE;
    } else {
      cerr << "Bad encoding: " << str << endl;
      exit(1);
    }
  }

  const char *encoding_to_str(encoding_t enc_type) {
    switch (enc_type) {
    case RLE:
      return "rle";
      break;
    case ARITHMETIC:
      return "arithmetic";
      break;
    case HUFFMAN:
      return "huffman";
      break;
    case NONE:
      return "none";
      break;
    default:
      throw runtime_error("Bad encoding_t");
    }    
  }

  std::ostream& operator<<(std::ostream& out, encoding_t enc_type) {
    return out << encoding_to_str(enc_type);
  }  
  

  ezw_header::ezw_header(size_t r, size_t c, int l, quantized_t m, unsigned long long s, quantized_t t, 
                         encoding_t et, size_t b, size_t p) 
    : rows(r), cols(c), level(l), mean(m), scale(s), threshold(t), enc_type(et), blocks(b), 
      passes(p), ezw_size(0), rle_size(0), enc_size(0)
  { 
    if (threshold && (threshold & (threshold-1))) {
      cerr << "Error: threshold is not power of 2: " << threshold << endl;
    }
  }
  
  size_t ezw_header::write_out(ostream& out) {
    size_t size = 0;

    size += vl_write(out, rows);
    size += vl_write(out, cols);
    size += vl_write(out, level);

    // need num_write for signed numbers
    size += write_generic(out, mean);
    size += vl_write(out, scale);
    
    // threshold is always a power of 2 or 0
    signed char log2_thresh = log2pow2(threshold);
    out.write((char*)&log2_thresh, 1);
    size += 1;

    unsigned char et = (unsigned char)enc_type;
    out.write((char*)&et, 1);
    size += 1;

    size += vl_write(out, blocks);
    size += vl_write(out, passes);

    size += vl_write(out, ezw_size);
    size += vl_write(out, rle_size);
    size += vl_write(out, enc_size);
    
    return size;
  }
  
  
  void ezw_header::read_in(istream& in, ezw_header& header) {
    header.rows = vl_read(in);
    header.cols = vl_read(in);
    header.level = vl_read(in);

    header.mean = read_generic<quantized_t>(in);
    header.scale = vl_read(in);

    signed char log2_thresh;
    in.read((char*)&log2_thresh, 1);
    
    if (log2_thresh < 0) {
      header.threshold = 0;
    } else {
      header.threshold = (1ll << log2_thresh);
    }

    unsigned char enc_type;
    in.read((char*)&enc_type, 1);
    header.enc_type = (encoding_t)enc_type;
    
    header.blocks = vl_read(in);
    header.passes = vl_read(in);

    header.ezw_size = vl_read(in);
    header.rle_size = vl_read(in);
    header.enc_size = vl_read(in);
  }


} // namespace
