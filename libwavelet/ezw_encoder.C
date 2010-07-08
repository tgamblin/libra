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
#include "ezw_encoder.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <stdexcept>
using namespace std;

#include "matrix_utils.h"
#include "wt_utils.h"
#include "io_utils.h"
#include "buffered_obitstream.h"
#include "vector_obitstream.h"
#include "rle.h"
#include "huffman.h"

//#define DEBUG
#ifdef DEBUG
#define DBG_OUT(x) (cerr << (x))
#else 
#define DBG_OUT(x)
#endif // DEBUG

namespace wavelet {

  ezw_encoder::ezw_encoder() : pass_limit(0), scale(1), enc_type(HUFFMAN) { }


  ezw_encoder::~ezw_encoder() { }


  // PRE: low_rows and low_cols are set.
  void ezw_encoder::build_zerotree_map() {
    // ensure matrices are the same size.
    if (zerotree_map.size1() != quantized.size1() || zerotree_map.size2() != quantized.size2()) {
      zerotree_map.resize(quantized.size1(), quantized.size2());
    }

    // copy quantized into zerotree map, replacing each w/largest power of 
    // two less than the magnitude
    for (size_t i=0; i < quantized.size1(); i++) {
      for (size_t j=0; j < quantized.size2(); j++) {
        zerotree_map(i,j) = lePowerOf2((uint64_t)abs_val(quantized(i,j)));
      }
    }

    // depth-first recursive encoding from each cell on the root level
    for (size_t r=0; r < low_rows; r++) {
      for (size_t c=0; c < low_cols; c++) {
        zerotree_map_encode(r, c);
      }
    }
  }


  quantized_t ezw_encoder::zerotree_map_encode(size_t r, size_t c) {
    // handle lowest frequency level case (3 children)
    if (r < low_rows && c < low_cols) {
      zerotree_map(r,c) |= zerotree_map_encode(r,          c+low_cols) 
        |                  zerotree_map_encode(r+low_rows, c         ) 
        |                  zerotree_map_encode(r+low_rows, c+low_cols);

      return zerotree_map(r,c);

    } else {
      // recursively process children
      size_t row = r << 1;
      size_t col = c << 1;
      
      if (row < quantized.size1() && col < quantized.size2()) {
        zerotree_map(r,c) |= zerotree_map_encode(row,   col  )
          |                  zerotree_map_encode(row,   col+1) 
          |                  zerotree_map_encode(row+1, col  ) 
          |                  zerotree_map_encode(row+1, col+1);
      }
      
      return zerotree_map(r,c);
    }
  }


  ezw_code ezw_encoder::encode_value(dom_elt e, obitstream& out) {
    quantized_t value = quantized(e.row, e.col);
    
    if (abs(value) >= threshold) {
      sub_list.push_back(abs(value));
      quantized(e.row, e.col) = 0;
      if (value >= 0) {
        out.put_one();
        out.put_one();
        DBG_OUT('p');
        return POSITIVE;
	
      } else {
        DBG_OUT('n');
        out.put_one(); 
        out.put_zero();
        return NEGATIVE;
      }
      
    } else if (threshold & zerotree_map(e.row, e.col)) {
      DBG_OUT('z');
      out.put_zero();
      out.put_one(); 
      return ZERO;
      
    } else {
      DBG_OUT('t');
      out.put_zero();
      out.put_zero();
      return ZERO_TREE;
    }
  }


  void ezw_encoder::subordinate_pass(obitstream& out) {
    for (size_t i=0; i < sub_list.size(); i++) {
      if ((sub_list[i] & threshold) != 0) {
        out.put_one();
        DBG_OUT(1);
      } else {
        out.put_zero();
        DBG_OUT(0);
      }
    }
  }


  void ezw_encoder::quantize(wt_matrix& mat, quantized_t scale) {
    if (quantized.size1() != mat.size1() || quantized.size2() != mat.size2()) {
      quantized.resize(mat.size1(), mat.size2());
    }
    
    for (size_t r=0; r < mat.size1(); r++) {
      for (size_t c=0; c < mat.size2(); c++) {
        quantized(r,c) = isnan(mat(r,c)) ? 0 : (quantized_t)round(mat(r,c) * scale);
      }
    }
  }


  void ezw_encoder::subtract_scalar(quantized_t scalar) {
    for (size_t r=0; r < quantized.size1(); r++) {
      for (size_t c=0; c < quantized.size2(); c++) {
        quantized(r,c) -= scalar;
      }
    }
  }


  void ezw_encoder::do_encode(obitstream& out, ezw_header& header, bool byte_align) {
    // Figure out bounds on the lowest transform level, so we can figure out
    // what kind of children we have.
    low_rows = quantized.size1() >> header.level;
    low_cols = quantized.size2() >> header.level;

    build_zerotree_map();

    dom_sizes.clear();
    sub_sizes.clear();

    encode_visitor visitor(this, out);

    while (threshold && (!pass_limit || (dom_sizes.size() < pass_limit))) {
      size_t start_bits = out.get_in_bits();

      dominant_pass(visitor, low_rows, low_cols, quantized.size1(), quantized.size2());
      size_t mid_bits = out.get_in_bits();

      DBG_OUT(endl);
      threshold >>= 1;
      if (threshold > 0) {
        subordinate_pass(out);
        DBG_OUT(endl);
      }
      
      // record number of bits in these passes
      dom_sizes.push_back(mid_bits - start_bits);
      sub_sizes.push_back(out.get_in_bits() - mid_bits);

      // IF we're byte-aligning passes, then we go ahead and output the last partial
      // byte.  If not, passes come one after the other.
      if (byte_align) {
        out.next_byte();      // force out trailing byte if it's there
      }
    }

    out.flush();            // force out trailing byte.
    sub_list.clear();         // cleanup for next call.
  }


  //TODO: make this method common to the coder and the wavelet transforms.
  int ezw_encoder::get_level(int level, size_t rows, size_t cols) {
    // for negative level, assume maximally transformed data as the transforms do.
    if (level < 1) {
      level = (int)log2pow2(max(rows, cols));
    }

    // for irregular sizes, ignore extra transforms in the longer direction.  Use
    // the level of the lowest frequency subband in the shorter direction to bound.
    if (level > (int)log2pow2(min(rows, cols))) {
      level = (int)log2pow2(min(rows, cols));
    }

    return level;
  }


  size_t ezw_encoder::encode(wt_matrix& mat, ostream& out, int level) {
    // First, compute values for header.
    level = get_level(level, mat.size1(), mat.size2());

    quantize(mat, scale);   // dump mat into quantized matrix

    // subtract out mean.
    quantized_t mean = (quantized_t)round(mean_val(quantized));
    subtract_scalar(mean);

    quantized_t abs_max = abs_max_val(quantized);
    threshold = lePowerOf2((uint64_t)abs_max);

    // construct and write out the header with relevant info
    ezw_header header(mat.size1(), mat.size2(), level, mean, scale, threshold, enc_type);

    vector_obitstream obits;
    do_encode(obits, header, false);
    obits.flush();

    header.ezw_size = obits.get_out_bytes();
    size_t enc_bytes = finish_encode(obits.get_vector(), out, header);
    return enc_bytes;
  }
  

  size_t ezw_encoder::finish_encode(vector<unsigned char>& buffer, ostream& out, ezw_header& header, bool rle) {
    size_t buf_size = header.ezw_size;
    
    // RLE encode everything first, unless it is already
    if (!rle) {
      const size_t rle_bound = (size_t)ceil(header.ezw_size * 257.0/256 + 1);
      vector<unsigned char> rle_buffer(rle_bound);

      header.rle_size = RLE_Compress(&buffer[0], &rle_buffer[0], buf_size);
      rle_buffer.resize(header.rle_size);
      rle_buffer.swap(buffer);
    }
    buf_size = header.rle_size;

    if (enc_type == HUFFMAN) {
      // --- Huffman code RLE buffer, then write out the results. --- //
      const size_t huff_bound = (size_t)ceil(buf_size * 101.0/100 + 384);
      vector<unsigned char> huff_buffer(huff_bound);
      header.enc_size = Huffman_Compress(&buffer[0], &huff_buffer[0], buf_size);
      huff_buffer.resize(header.enc_size);
      huff_buffer.swap(buffer);
      buf_size = header.enc_size;

      const size_t header_size = header.write_out(out);

      out.write((char*)&buffer[0], buf_size);
      return header_size + buf_size;

    } else if (enc_type == ARITHMETIC) {
      // --- Arithmetic coding is done via streams, so handle slightly differently --- //
      header.enc_size = 0; // this is streaming, so enc_size is unknown.

      const size_t header_size = header.write_out(out);
      ac_obitstream ac_out(out);
      ac_out.write_bits(&buffer[0], (header.rle_size << 3));
      ac_out.flush();
      return header_size + bits_to_bytes(ac_out.get_out_bits());

    } else {
      // --- If no huffman or arithmetic coding, just write out the buffer here. --- //
      const size_t header_size = header.write_out(out);
      out.write((char*)&buffer[0], buf_size);
      return header_size + buf_size;
    }
  }


  int ezw_encoder::get_pass_limit() {
    return pass_limit;
  }

  void ezw_encoder::set_pass_limit(size_t limit) {
    pass_limit = limit;
  }


  quantized_t ezw_encoder::get_scale() {
    return scale;
  }

  void ezw_encoder::set_scale(quantized_t s) {
    scale = s;
  }

  encoding_t ezw_encoder::get_encoding_type() {
    return enc_type;
  }

  void ezw_encoder::set_encoding_type(encoding_t type) {
    if (type == NONE) {
      throw runtime_error("Error: invalid encoding.");
    }
    enc_type = type;
  }

} // namespace

