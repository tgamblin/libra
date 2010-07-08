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
#ifndef VTK_EFFORT_DATA_H
#define VTK_EFFORT_DATA_H

#include "vtkObject.h"
#include "EffortData.h"
#include "wavelet.h"
using wavelet::wt_matrix;

/// This class represents data and metadata from a particular effort
/// region monitored at runtme.  Each effort region 
class vtkEffortData : public vtkObject {
public:
  static vtkEffortData *New();
  vtkTypeRevisionMacro(vtkEffortData, vtkObject);

  /// Sets the effort region this will delegate to.
  virtual void setEffortData(EffortData *del) {
    delegate = del;
  }

  /// Get data matrix.  This will lazily load data from the file.
  const wt_matrix& getData() {
    return delegate->getData();
  }
  
  /// Get coefficients matrix.  Lazily loads data from file.
  const wt_matrix& getCoefficients() {
    return delegate->getCoefficients();
  }
  
protected:
  vtkEffortData();
  virtual ~vtkEffortData();

private:
  vtkEffortData(const vtkEffortData& other);            // not implemented
  vtkEffortData& operator=(const vtkEffortData& other); // not implemented

  EffortData *delegate;
};


#endif // VTK_EFFORT_DATA_H
