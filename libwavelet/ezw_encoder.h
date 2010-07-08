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
#ifndef WT_EZW_ENCODER_H
#define WT_EZW_ENCODER_H

#include <deque>
#include <vector>
#include <climits>

#include "ezw.h"
#include "wavelet.h"
#include "obitstream.h"

namespace wavelet {

  /// This class provides methods for encoding wavelet matrices 
  /// using Shapiro's EZW method.
  class ezw_encoder {
  public:
    /// Constructor instantiates an ezw coder and all the storage it
    /// needs for encoding.
    /// Params:
    /// scale: Factor to scale doubles in input by before coding
    ///        This preserves fractional values.  Defaults to (1 << 20) 
    ///         bits, or about 6 decimal digits.
    ezw_encoder();

    /// Destructor; just deallocates everything.
    virtual ~ezw_encoder();

    /// Takes a matrix full of wavelet coefficients and encodes it
    /// with the EZW algorithm.  Output is written onto the supplied
    /// stream.  
    /// 
    /// Params:
    /// mat         Wavelet-transformed input data.
    /// out         Output stream to write encoded data to.
    /// level       Level of the wavelet transform that was applied to the 
    ///             input data.  Assumes maximal if not provided.
    /// scale       Factor to multiply data by before encoding.
    /// byte_budget Max number of bytes allowed in output.
    /// 
    /// Return value:
    ///     Number of bytes written out.
    size_t encode(wt_matrix& mat, std::ostream& out, int level = -1);
    
    /// Number of EZW passes to encode; 0 for no limit.
    int get_pass_limit();

    /// Sets number of EZW passes to encode; 0 for no limit.
    void set_pass_limit(size_t limit);
    
    /// Scaling factor by which doubles are multiplied before being quantized.
    quantized_t get_scale();

    /// Sets scaling factor by which doubles are multiplied before being quantized.
    void set_scale(quantized_t scale);

    /// Whether encoder uses arithmetic coding (default is true)
    encoding_t get_encoding_type();
    
    /// Set whether to use arithmetic coding (default is true)
    void set_encoding_type(encoding_t enc_type);

  protected:
    /// Values from input matrix, quantized.
    boost::numeric::ublas::matrix<quantized_t> quantized;

    /// map of zero trees for encoding step
    boost::numeric::ublas::matrix<quantized_t> zerotree_map;

    size_t low_rows;                   /// Rows in lowest frequency pass
    size_t low_cols;                   /// Cols in lowest frequency pass

    size_t pass_limit;                 /// Max number of EZW passes to output
    quantized_t scale;                 /// pre-transform scaling factor.
    encoding_t enc_type;               /// Type of encoding for output.  Defaults to huffman.

    /// Number of bits in each ezw pass (used by parallel version)
    std::vector<size_t> dom_sizes;
    std::vector<size_t> sub_sizes;

    quantized_t threshold;              /// Current threshold for the coder.   
    std::vector<quantized_t> sub_list;  /// accumulated subordinate pass coefficients


    /// EZW-codes a single value according to the current threshold.  
    /// Appends to dom_queue, and sub_list if necessary.
    ezw_code encode_value(dom_elt e, obitstream& out);

    /// Subordinate pass of EZW algorithm.  ee Shapiro, 1993 for info.
    void subordinate_pass(obitstream& out);

    /// gets level of transform based on size of matrix.
    int get_level(int level, size_t rows, size_t cols);

    /// Multiplies each value in the matrix by a scale factor then casts it to quantized_t.
    /// Stored results in an internal matrix of quantized values.
    void quantize(wt_matrix& mat, quantized_t scale);
    
    /// Build zerotree map.  Map is constructed from quantized and stored in zerotree_map.
    /// Threshold can be simply ANDed with zerotree_map values to determine if a cell is a 
    /// zerotree root.
    /// See Shapiro 1996, "A Fast Technique for Finding Zerotrees in the EZW Algorithm".
    void build_zerotree_map();

    /// Recursive helper for build_zerotree_map().  
    quantized_t zerotree_map_encode(size_t r, size_t c);

    /// Subtracts the provided scalar value from the entire quantized matrix.
    void subtract_scalar(quantized_t scalar);
    
    /// Does the actual work of the EZW algorithm; used by both sequential and parallel
    /// calls above.
    /// 
    /// PRE: threshold has been set according to max value in array.
    /// 
    /// Params:
    /// out         Output stream to write encoded data to.
    /// level       Header containing parameters of the transform to be performed.
    /// byte_align  If true, beginning of each pass is aligned to bytes in the output.
    /// 
    /// Return value:
    ///     Number of bytes written to output stream by the encoder
    void do_encode(obitstream& out, ezw_header& header, bool byte_align);


    /// Finishes encoding by coding buf and writing out to file.
    /// Returns size of encoded output data, not including header size.
    /// If pre_rle is specified the buffer is assumed to already be rle coded.
    size_t finish_encode(std::vector<unsigned char>& buf, std::ostream& out, ezw_header& header, bool rle = false);


    /// Used by dominant pass to encode valus in a bitstream.  See
    /// ezw.h for traversals in which this can be used.
    struct encode_visitor {
      ezw_encoder *parent;
      obitstream& out;
      
      encode_visitor(ezw_encoder *p, obitstream& o): parent(p), out(o) { }
      ~encode_visitor() { }
      ezw_code visit(dom_elt e) { 
        return parent->encode_value(e, out);
      }
    };

  };

} // namespace

#endif // WT_EZW_ENCODER_H
