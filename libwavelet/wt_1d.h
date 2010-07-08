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
#ifndef WT_1D_H
#define WT_1D_H

#include "wavelet.h"

namespace wavelet {
  
  class wt_1d {
  public:
    wt_1d() { }
    virtual ~wt_1d() { }

    /// Algorithm for forward transform in 1 dimension.  Applies wavelet transform on
    /// lower-frequency bands recursively up to level, or as far as possible if level is -1.
    virtual int fwt_1d(double * data, size_t len, int level = -1);
    
    /// Algorithm for inverse transform in 1 dimension.  Starts at fwt_level and applies 
    /// inverse transform recursively up to iwt_level times.
    /// 
    /// fwt_level:    level of the fwt applied to the matrix (default max possible)
    /// iwt_level:    level of iwt to perform on the matrix. (defaults to fwt_level)
    virtual int iwt_1d(double *data, size_t len, int fwt_level = -1, int iwt_level = -1);

  protected:
    // single-level implementations of the wavelet transform.
    virtual void fwt_1d_single(double *data, size_t len) = 0;
    virtual void iwt_1d_single(double *data, size_t len) = 0;

  }; // wt_1d
  
} // namespace wavelet

#endif // WT_1D_H
