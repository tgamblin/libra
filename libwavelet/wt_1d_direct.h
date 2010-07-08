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
#ifndef WT_1D_DIRECT_H
#define WT_1D_DIRECT_H

#include <vector>
#include "wt_1d.h"
#include "filter_bank.h"
#include "cdf97.h"

namespace wavelet {
  
  /// This class provides functions for doing 1d, direct wavelet transforms
  /// on contiguous data.  Also provides internals for filters and an internal 
  /// buffer to store data being transformed.
  ///
  /// This is designd to be inherited privately by something more sophisticated
  /// that can make use of its functions. e.g. wt_direct. which knows about 
  /// boost matrices.
  class wt_1d_direct : public wt_1d {
  public:
    /// Constructs a new direct wavelet transform with the provided filter bank.
    /// Filter defaults to CDF 9/7 Wavelets.
    wt_1d_direct(filter_bank& f = filter::getCDF97());

    /// Destructor
    virtual ~wt_1d_direct();

    /// Forward transform for raw contiguous data.
    virtual void fwt_1d_single(double *data, size_t n);

    /// Inverse transform for raw contiguous data.
    virtual void iwt_1d_single(double *data, size_t n);

  protected:
    /// Filter bank for this transform
    filter_bank& f;

    /// temporary storage for packing values
    std::vector<double> temp;   

    /// Copies x into temp and symmetrically extends edges by filter size.
    ///   e.g. if filter size is 3 and x is:
    ///            1 2 3 4 5 6 7
    ///   temp is filled with:
    ///      4 3 2 1 2 3 4 5 6 7 6 5 4
    /// Params:
    ///      x   input array data
    ///      n   elements to copy from input
    /// stride   stride of data in input
    void sym_extend(double *x, size_t n, size_t stride = 1, bool interleave = false);
  };

} // namespace

#endif // WT_1D_DIRECT_H
