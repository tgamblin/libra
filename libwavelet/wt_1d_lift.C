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
#include "wt_1d_lift.h"

namespace wavelet {

    // TODO: make lifting filters pluggable.  Export this filter.
  
  /// Lifting filter for Cohen-Daubechies-Feauveau 9/7 wavelet
  static const double lift_filter[] = {
    -1.5861343420693648,
    -0.0529801185718856,
    0.8829110755411875,
    0.4435068520511142
  };
  
  /// Scaling factor used in transforms below.
  static const double scale_factor = 1.1496043988602418;


  void wt_1d_lift::fwt_1d_single(double *data, size_t n) {
    double a;
    size_t i;
    
    // Predict 1
    a=lift_filter[0];
    for (i=1;i<n-2;i+=2) {
      data[i]+=a*(data[i-1]+data[i+1]);
    } 
    data[n-1]+=2*a*data[n-2];
    
    // Update 1
    a=lift_filter[1];
    for (i=2;i<n;i+=2) {
      data[i]+=a*(data[i-1]+data[i+1]);
    }
    data[0]+=2*a*data[1];
    
    // Predict 2
    a=lift_filter[2];
    for (i=1;i<n-2;i+=2) {
      data[i]+=a*(data[i-1]+data[i+1]);
    }
    data[n-1]+=2*a*data[n-2];
    
    // Update 2
    a=lift_filter[3];
    for (i=2;i<n;i+=2) {
      data[i]+=a*(data[i-1]+data[i+1]);
    }
    data[0]+=2*a*data[1];

    // Scale
    a=1/scale_factor;
    for (i=0;i<n;i++) {
      if (i%2) data[i]*=a;
      else data[i]/=a;
    }

    // Pack
    if (temp.size() < n) temp.resize(n);
    for (i=0;i<n;i++) {
      if (i%2==0) temp[i/2] = data[i];
      else temp[n/2+i/2] = data[i];
    }
    for (i=0;i<n;i++) data[i] = temp[i];
  }


  void wt_1d_lift::iwt_1d_single(double *data, size_t n) {
    double a;
    size_t i;

    // Unpack
    if (temp.size() < n) temp.resize(n);
    for (i=0;i<n/2;i++) {
      temp[i*2]=data[i];
      temp[i*2+1]=data[i+n/2];
    }
    for (i=0;i<n;i++) data[i] = temp[i];

    // Undo scale
    a=scale_factor;
    for (i=0;i<n;i++) {
      if (i%2) data[i]*=a;
      else data[i]/=a;
    }

    // Undo update 2
    a=-lift_filter[3];
    for (i=2;i<n;i+=2) {
      data[i]+=a*(data[i-1]+data[i+1]);
    }
    data[0]+=2*a*data[1];

    // Undo predict 2
    a=-lift_filter[2];
    for (i=1;i<n-2;i+=2) {
      data[i]+=a*(data[i-1]+data[i+1]);
    }
    data[n-1]+=2*a*data[n-2];

    // Undo update 1
    a=-lift_filter[1];
    for (i=2;i<n;i+=2) {
      data[i]+=a*(data[i-1]+data[i+1]);
    }
    data[0]+=2*a*data[1];

    // Undo predict 1
    a=-lift_filter[0];
    for (i=1;i<n-2;i+=2) {
      data[i]+=a*(data[i-1]+data[i+1]);
    } 
    data[n-1]+=2*a*data[n-2];
  }



} // namespace wavelet
