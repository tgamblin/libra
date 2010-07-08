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
#ifndef WT_EZW_H
#define WT_EZW_H

#include <climits>
#include <deque>
#include "ac_obitstream.h"
#include "buffered_obitstream.h"
#include "ac_ibitstream.h"
#include "buffered_ibitstream.h"

namespace wavelet {

  /// All input data is converted to this type before coding.
  typedef long long quantized_t;  
  
  /// Minimum value of a quantized_t; be sure to keep this synced.
  const quantized_t Q_MIN = LONG_LONG_MIN;  
  
  /// Possible types of encoding to use for encoding after rle-encoding ezw data 
  typedef enum { NONE, RLE, HUFFMAN, ARITHMETIC } encoding_t;

  /// Helpful for input
  encoding_t str_to_encoding(const char *str);
  const char *encoding_to_str(encoding_t enc_type);
  std::ostream& operator<<(std::ostream& out, encoding_t enc_type);  
  
  /// This is all the data needed by the decoder to parse the encoder's output.
  struct ezw_header {
    // initialized fields (set in constructor)
    size_t rows;               // Rows in encoded matrix
    size_t cols;               // Cols in encoded matrix
    size_t level;              // Level of wavelet transform and ezw coding done on data
    quantized_t mean;          // Mean of data in this file (subtracted out before encoding)
    unsigned long long scale;  // Scaling factor applied to data before encoding
    quantized_t threshold;     // Initial ezw threshold for data in this file.
    encoding_t enc_type;       // Type of encoding used on rle buffer.
    size_t blocks;             // For parallel encoding -- count of independently encoded blocks
    size_t passes;             // Needed for block coding: total number of ezw passes encoded.

    // un-initialized fields (must be set manually)
    size_t ezw_size;        // Size of ezw-encoded bitstream
    size_t rle_size;        // Size of ezw after rle coding
    size_t enc_size;        // Size of fully encoded rle buffer

    ezw_header() { }

    ezw_header(size_t r, size_t c, int l, quantized_t m, unsigned long long s, quantized_t t, 
               encoding_t et = ARITHMETIC, size_t b = 1, size_t p = 0);
    
    size_t write_out(std::ostream& out);
    static void read_in(std::istream& in, ezw_header& header);
  };

  std::ostream& operator<<(std::ostream& out, const ezw_header& header);


  /// These are the symbols output from the dominant pass.  
  /// POSITIVE or NEGATIVE: coeff magnitude was greater than threshold.
  /// ZERO_TREE: coeff and all descendants were less than threshold.
  /// ZERO: coeff was less than threshold, but not all descendants were.
  /// STOP: this is a control code used by the traversal functions.  Returning
  ///       this from a visitor signals for the traversal to abort and return 
  ///       to its caller.
  typedef enum { 
    POSITIVE, NEGATIVE, ZERO_TREE, ZERO, STOP
  } ezw_code;


  /// These go in the subordinate pass's list
  struct sub_elt {
    size_t row;
    size_t col;
    sub_elt(size_t r, size_t c) : row(r), col(c) { }
  };

  /// These go in the dominant pass's list
  struct dom_elt {
    size_t row;
    size_t col;
    int level;
    dom_elt(size_t r, size_t c, int l) : row(r), col(c), level(l) { }
  };
  
  
  template <class Visitor>
  bool morton_traversal(Visitor visitor, size_t low_rows, size_t low_cols, size_t rows, size_t cols) {
    std::deque<dom_elt> dom_queue;
    
    // queue up work for lowest frequency level
    for (size_t r=0; r < low_rows; r++) {
      for (size_t c=0; c < low_cols; c++) {
        dom_queue.push_back(dom_elt(r, c, 0));
      }
    }
    
    // clear out lowest frequency work, as it's all done.
    while (!dom_queue.empty()) {
      dom_elt e = dom_queue.front();
      dom_queue.pop_front();
      ezw_code code = visitor.visit(e);

      if (code == STOP) return false;

      if (e.level == 0) {
        // put children of level zero values on the queue
        dom_queue.push_back(dom_elt(e.row,          e.col+low_cols, 1));
        dom_queue.push_back(dom_elt(e.row+low_rows, e.col,          1));
        dom_queue.push_back(dom_elt(e.row+low_rows, e.col+low_cols, 1));
	
      } else if (code != ZERO_TREE) {
        // put children of this value on the queue.
        size_t row = e.row << 1;
        size_t col = e.col << 1;
        int next_level = e.level + 1;
	
        if (row < rows && col < cols) {
          dom_queue.push_back(dom_elt(row,   col,   next_level));
          dom_queue.push_back(dom_elt(row,   col+1, next_level));
          dom_queue.push_back(dom_elt(row+1, col,   next_level));
          dom_queue.push_back(dom_elt(row+1, col+1, next_level));
        }
      }
    }
    return true;
  }
  

  template <class Visitor>
  bool depth_first_traversal(Visitor visitor, size_t low_rows, size_t low_cols, size_t rows, size_t cols,
                             size_t num_blocks, size_t block) {
    std::deque<dom_elt> dom_queue;

    long long start_row = (low_rows / num_blocks) * block;
    long long end_row = start_row + (low_rows / num_blocks);
    
    // queue up work for lowest frequency level
    for (long long r=end_row-1; r >= start_row; r--) {
      for (long long c=low_cols-1; c >= 0; c--) {
        dom_queue.push_back(dom_elt(r, c, 0));
      }
    }
    
    // clear out lowest frequency work, as it's all done.
    while (!dom_queue.empty()) {
      dom_elt e = dom_queue.back();
      dom_queue.pop_back();

      ezw_code code = visitor.visit(e);
      if (code == STOP) return false;

      if (e.level == 0) {
        // put children of level zero values on the queue
        dom_queue.push_back(dom_elt(e.row+low_rows, e.col+low_cols, 1));
        dom_queue.push_back(dom_elt(e.row+low_rows, e.col,          1));
        dom_queue.push_back(dom_elt(e.row,          e.col+low_cols, 1));

      } else if (code != ZERO_TREE) {
        // put children of this value on the queue.
        size_t row = e.row << 1;
        size_t col = e.col << 1;
        int next_level = e.level + 1;
	
        if (row < rows && col < cols) {
          dom_queue.push_back(dom_elt(row+1, col+1, next_level));
          dom_queue.push_back(dom_elt(row+1, col,   next_level));
          dom_queue.push_back(dom_elt(row,   col+1, next_level));
          dom_queue.push_back(dom_elt(row,   col,   next_level));
        }
      }
    }
    return true;
  }


  /// Dominant pass of EZW algorithm.  Needs to do the same traversal in the
  /// encoder and the decoder.  Both call this method.
  /// See Shapiro, 1993 for info.
  ///
  /// visotor:    callable object to apply to all elts
  /// low_rows:   rows in lowest-frequency pass
  /// los_cols:   cols in lowest-frequency pass
  /// rows:       rows in encoded data
  /// cols:       cols in encoded data
  /// num_blocks: total number of separately-encoded block in data
  /// block:      id of current block to decode (depth-first only)
  /// 
  template <class Visitor>
  bool dominant_pass(Visitor visitor, size_t low_rows, size_t low_cols, size_t rows, size_t cols,
                     size_t num_blocks = 1, size_t block = 0) {
    //return morton_traversal(visitor, low_rows, low_cols, rows, cols);
    return depth_first_traversal(visitor, low_rows, low_cols, rows, cols, num_blocks, block);
  }

  
} // namespace

#endif // WT_EZW_H
