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
#include "summary.h"
using namespace wavelet;

#include <cmath>
#include <cfloat>
#include <vector>
using namespace std;

#include "matrix_utils.h"

void Summary::set_matrix(const wavelet::wt_matrix& new_matrix) {
  m = &new_matrix;
  computed = false;
  start_row = 0;
  start_col = 0;
  end_row = m->size1();
  end_col = m->size2();
}


void Summary::compute() {
  const wt_matrix& mat = *m;
  assert(in_bounds(mat, start_row, start_col));
  assert(in_bounds(mat, end_row-1, end_col-1));
  
  mTotal = 0;
  mCount = 0;
  mMean = 0;
  mMax = -DBL_MAX;
  mMin = DBL_MAX;

  size_t rows = end_row - start_row;
  vector<double> row_mean(rows);
  size_t rmx = 0;

  for (size_t i=start_row; i < end_row; i++) {
    double row_total = 0;
    for (size_t j=start_col; j < end_col; j++) {
      row_total += mat(i,j);
      mMax = ::max(mMax, mat(i,j));
      mMin = ::min(mMin, mat(i,j));
      mCount++;
    }
    
    mTotal += row_total;
    row_mean[rmx++] = row_total / (end_col - start_col);
  }
  
  mMean = mTotal / mCount;
  
  rmx = 0;
  double totalRowVariance = 0;
  double totalRowSkew = 0;
  double totalRowKurtosis = 0;
  mMinRowVariance = mMinRowSkew = mMinRowKurtosis = DBL_MAX;
  mMaxRowVariance = mMaxRowSkew = mMaxRowKurtosis = -DBL_MAX;
  
  for (size_t i=start_row; i < end_row; i++) {  
    double sumSquares = 0;
    double sumCubes = 0;
    double sumQuads = 0;
    
    for (size_t j=start_col; j < end_col; j++) {
      double diff = mat(i,j) - row_mean[rmx];
      double diff2 = diff * diff;

      sumSquares += diff2;
      sumCubes   += diff2 * diff;
      sumQuads   += diff2 * diff2;
    }

    size_t cols = end_col - start_col;
    double rowVariance = sumSquares / (cols - 1);

    double rowStdDev = sqrt(rowVariance);
    double rowStdDev3 = rowStdDev * rowStdDev * rowStdDev;
    double rowStdDev4 = rowStdDev3 * rowStdDev;

    double rowSkew = sumCubes / ((cols - 1) * rowStdDev3);
    double rowKurtosis = sumQuads / ((cols - 1) * rowStdDev4);
    
    totalRowVariance += rowVariance;
    totalRowSkew     += rowSkew;
    totalRowKurtosis += rowKurtosis;
    
    mMinRowVariance = ::min(rowVariance, mMinRowVariance);
    mMinRowSkew     = ::min(rowSkew,     mMinRowSkew);
    mMinRowKurtosis = ::min(rowKurtosis, mMinRowKurtosis);

    mMaxRowVariance = ::max(rowVariance, mMaxRowVariance);
    mMaxRowSkew     = ::max(rowSkew,     mMaxRowSkew);
    mMaxRowKurtosis = ::max(rowKurtosis, mMaxRowKurtosis);
    
    rmx++;
  }
  
  mMeanRowVariance = totalRowVariance / rows;
  mMeanRowSkew     = totalRowSkew     / rows;
  mMeanRowKurtosis = totalRowKurtosis / rows;

  mMeanRowDeviation = sqrt(mMeanRowVariance);
  mMinRowDeviation  = sqrt(mMinRowVariance);
  mMaxRowDeviation  = sqrt(mMaxRowVariance);
}
