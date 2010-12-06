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
#include "wt_2d.h"

#include <stdint.h>
#include <climits>
#include <iostream>

#include "io_utils.h"

using namespace std;

namespace wavelet {

  int wt_2d::fwt_2d(wt_matrix& mat, int level) {
    if (level < 0) {
      level = (int)log2pow2(std::max(mat.size1(), mat.size2()));
    }
    assert(level <= log2pow2(std::max(mat.size1(), mat.size2())));
  
    size_t rows = mat.size1();
    size_t cols = mat.size2();
    for (int i=0; i < level; i++) {
      if (cols > 1) for (size_t r=0; r < rows; r++) fwt_row(mat, r, cols);
      if (rows > 1) for (size_t c=0; c < cols; c++) fwt_col(mat, c, rows);

      if (rows > 1) rows >>= 1;
      if (cols > 1) cols >>= 1;
    }

    return level;
  }


  int wt_2d::iwt_2d(wt_matrix& mat, int fwt_level, int iwt_level) {
    if (fwt_level < 0) {
      fwt_level = (int)log2pow2(std::max(mat.size1(), mat.size2()));
    }
    assert(fwt_level <= log2pow2(std::max(mat.size1(), mat.size2())));

    if (iwt_level < 0) {
      iwt_level = INT_MAX;
    }
      
    size_t rows, cols;
    int levels = 0;
    for (int i=fwt_level-1; i >= 0 && levels < iwt_level; i--) {
      rows = mat.size1() >> i;
      if (!rows) rows = 1;

      cols = mat.size2() >> i;
      if (!cols) cols = 1;
      
      if (rows > 1) for (size_t c=0; c < cols; c++) iwt_col(mat, c, rows);
      if (cols > 1) for (size_t r=0; r < rows; r++) iwt_row(mat, r, cols);

      levels++;
    }

    return levels;
  }


} // namespace wavelet
	
