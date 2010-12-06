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
#include "wt_1d.h"

#include <climits>

#include "io_utils.h"
using namespace std;

namespace wavelet {

  int wt_1d::fwt_1d(double * data, size_t len, int level) {
    if (level < 0) {
      level = (int)log2pow2(len);
    }
    assert(level <= log2pow2(len));

    size_t cur_len = len;
    for (int i=0; i < level; i++) {
      fwt_1d_single(data, cur_len);
      cur_len >>= 1;
    }

    return level;    
  }
  

  int wt_1d::iwt_1d(double *data, size_t len, int fwt_level, int iwt_level) {
    if (fwt_level < 0) {
      fwt_level = (int)log2pow2(len);
    }
    assert(fwt_level <= log2pow2(len));

    if (iwt_level < 0) {
      iwt_level = INT_MAX;
    }
      
    int levels = 0;
    for (int i=fwt_level-1; i >= 0 && levels < iwt_level; i--) {
      size_t cur_len = len >> i;
      iwt_1d_single(data, cur_len);
      levels++;
    }

    return levels;    
  }

} // namespace wavelet
