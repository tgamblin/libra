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
#ifndef LIFT_WT_2D_H
#define LIFT_WT_2D_H

#include <vector>
#include "wavelet.h"
#include "wt_1d_lift.h"
#include "wt_2d.h"

namespace wavelet { 

  /// This is a lifting implementation of the CDF 9/7 wavelet transform.
  /// Matrices passed in must be 
  /// TODO: modify column routines to use groups for speed.
  /// TODO: arbitrarily-sized matrices.
  ///
  /// by Todd Gamblin October 25, 2007.
  ///
  class wt_lift : public wt_2d, public wt_1d_lift {
  public:
    /// Default Constructor
    wt_lift();
    
    /// Destructor
    virtual ~wt_lift();

    /// Forward wavelet transform for matrix rows.
    virtual void fwt_row(wt_matrix& mat, size_t row, size_t n) {
      fwt_1d_single(&mat(row, 0), n);
    }

    /// Forward wavelet transform for matrix cols
    virtual void fwt_col(wt_matrix& mat, size_t col, size_t n);


    /// Inverse wavelet transform for matrix rows.
    virtual void iwt_row(wt_matrix& mat, size_t row, size_t n) {
      iwt_1d_single(&mat(row, 0), n);
    }

    /// Inverse wavelet transform for matrix cols
    virtual void iwt_col(wt_matrix& mat, size_t col, size_t n);
  };

} // namespace 

#endif //LIFT_WT_2D_H
