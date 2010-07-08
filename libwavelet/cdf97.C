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
#include "cdf97.h"

#include <iostream>
#include <cstdlib>
#include <cmath>
using namespace std;

namespace wavelet { namespace filter {

  // static data is function-static here to avoid library initialization issues.
  filter_bank& getCDF97() {
    static filter_bank *cdf97 = NULL;

    /// lpf coefficients
    static const double l[5] = {
      0.026748757411,
      -0.016864118443,
      -0.078223266529,
      0.266864118443,
      0.602949018236
    };
    
    /// hpf coefficients
    static const double h[5] = {
      0.0,
      0.045635881557,
      -0.028771763114,
      -0.295635881557,
      0.557543526229
    };
    
    static const double lpf[9] = {
      l[0], l[1], l[2], l[3], l[4], l[3], l[2], l[1], l[0]
    };
    
    static const double hpf[9] = {
      h[0], h[1], h[2], h[3], h[4], h[3], h[2], h[1], h[0]
    };
    
    static const double ilpf[9] = {
      h[0], -h[1], h[2], -h[3], h[4], -h[3], h[2], -h[1], h[0]
    };
    
    static const double ihpf[9] = {
      l[0], -l[1], l[2], -l[3], l[4], -l[3], l[2], -l[1], l[0]
    };
    
    if (!cdf97) {
      const int size = 9;
      cdf97 = new filter_bank(size, lpf, hpf, ilpf, ihpf);
      
      // apply some scaling here to get this to agree with lifting implementation
      for (int i=0; i < size; i++) {
	cdf97->lpf[i]  *= sqrt(2.0);
	cdf97->hpf[i]  *= sqrt(2.0);
	// output of transform is decimated by 2, so need to multiply iwt filters by 2
	cdf97->ilpf[i] *= 2.0/sqrt(2.0);
	cdf97->ihpf[i] *= 2.0/sqrt(2.0);
      }
    }
    return *cdf97;
  }
  
}} // namespace
  
