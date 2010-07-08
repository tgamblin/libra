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
#include "wt_1d_direct.h"

#include <cassert>
using namespace std;

namespace wavelet {

  // just inits the filter.
  wt_1d_direct::wt_1d_direct(filter_bank& filter) : f(filter) { }
  
  // currently does nothing.
  wt_1d_direct::~wt_1d_direct() { }


  void wt_1d_direct::sym_extend(double *x, size_t n, size_t stride, bool interleave) {
    size_t tsize = n + (2 * (f.size/2) + 1);
    if (tsize > temp.size()) temp.resize(tsize);

    // copy data from x into middle of temp
    if (interleave) {
      // this interleaves first and second half of x in temp
      for (size_t i=0; i < n/2; i++) {
        temp[f.size/2+(2*i)] = x[i*stride];
        temp[f.size/2+(2*i+1)] = x[(n/2+i)*stride];
      }

    } else {
      // this just copies x straight into temp
      for (size_t i=0; i < n; i++) {
        temp[f.size/2+i] = x[i*stride];
      }
    }

    // symmetrically extend left and right border around data
    int l = f.size/2-1;
    int r = n + f.size/2;
    for (size_t i=1; i<=f.size/2; i++) {
      temp[l] = temp[l+2*i];
      temp[r] = temp[l+n-1];
      l--;
      r++;
    }
    temp[r] = temp[l+n-1];   // last elt on right
  }


  void wt_1d_direct::fwt_1d_single(double *data, size_t n) {
    assert(!(n & 1));

    sym_extend(data, n, 1);
    size_t len = n >> 1;
    for (size_t i=0; i < len; i++) {
      data[i] = data[len+i] = 0;

      for (size_t d=0; d < f.size; d++) {
        data[i] += f.lpf[d] * temp[2*i+d];
        data[len+i] += f.hpf[d] * temp[2*i+d+1];
      }
    }
  }

  
  void wt_1d_direct::iwt_1d_single(double *data, size_t n) {
    assert(!(n & 1));

    sym_extend(data, n, 1, true);
    for (size_t i=0; i < n; i++) {
      data[i] = 0.0;
      for (size_t d=0; d < f.size; d++) {
        // this check upsamples the two bands in the input data
        // sym_extend packs the data interleaved; we just skip even/odd indices
        // here instead of having 2 extra temp arrays for the upsampled data.
        if ((i+d) & 1) data[i] += f.ihpf[d] * temp[i+d];
        else           data[i] += f.ilpf[d] * temp[i+d];
      }
    }
  }

} // wavelet
