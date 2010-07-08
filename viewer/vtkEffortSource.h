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
#ifndef VTK_EFFORT_SOURCE_H
#define VTK_EFFORT_SOURCE_H

#include "vtkWarpScalar.h"
#include "wavelet.h"
class vtkEffortData;
class vtkPlaneSource;
class vtkTransform;
class vtkTransformPolyDataFilter;
class vtkProgrammableFilter;
using wavelet::wt_matrix;

/// This class is a data source for point values from Effort functions.
/// It starts out as an xy plane, but you can add vtkEffortData to it to 
/// warp the surface according to performance data.
class vtkEffortSource : public vtkWarpScalar {
public:
  static vtkEffortSource *New();

  vtkTypeRevisionMacro(vtkEffortSource, vtkWarpScalar);

  /// Adds effort data to the effort function.  Data is initially zero.
  void AddEffort(vtkEffortData *effort);

  /// Number of rows in the underlying dataset.
  virtual int Rows();
  
  /// Number of columns in the underlying dataset.
  virtual int Cols();

  /// Get a particular value by row/column from the dataset
  virtual double GetValue(int row, int col);  

  /// Get a particular value scaled by scaleFactor
  virtual double GetNormalizedValue(int row, int col);  

  /// Gets the range of rows that should be output
  /// Range includes the start but excludes the end.
  virtual int *GetRowRange();

  /// Gets the range of columns that should be output
  /// Range includes the start but excludes the end.
  virtual int *GetColRange();

  /// Sets the range of rows that should be output.  
  /// Range includes the start but excludes the end.
  virtual void SetRowRange(int start, int end);

  /// Sets the range of cols that should be output.  
  /// Range includes the start but excludes the end.
  virtual void SetColRange(int start, int end);

  /// Returns range of actual data for the function (GetDims returns normalized range).
  /// Same as GetDims if not normalized.
  virtual double *GetBounds();

  /// Returns normalized range, if normalization is enabled.  Otherwise same as GetRange()
  virtual double *GetNormalizedBounds();

  /// Controls the behavior of the function.  If true, z values are normalized
  /// to span the minimum of the x and y ranges.
  virtual void SetNormalize(int normalize);

  /// Return scaling ratio
  virtual double GetScaleRatio();

  /// Set scaling ratio
  virtual void SetScaleRatio(double ratio);

  /// Set whether this adds wavelet coefficients or spatial domain coeffs.
  /// Note parameter needs to be int (not bool) for VTK wrapper generator to work right.
  virtual void SetRenderWavelets(int render);

protected:
  vtkEffortSource();
  virtual ~vtkEffortSource();

private:
  wt_matrix mat;        /// internal data from all effort added to this function
  double scale_ratio;   /// Ratio of max Z axis to min(X,Y) to use for normalizing height.  Default .25

  bool empty;           /// True if nothing has been added yet.
  bool is_wavelet;      /// True if we should add wavelet coefficients instead of spatial
  bool normalize;       /// True if we should normalize values returned by EvaluateFunction to some reasonable range.

  size_t x_min;
  size_t x_max;
  size_t y_min;
  size_t y_max;
  double z_min;
  double z_max;

  double bounds[6];
  double normalized_bounds[6];

  int row_range[2];
  int col_range[2];

  vtkPlaneSource *plane;
  vtkTransform *transform;
  vtkTransformPolyDataFilter *transF;
  vtkProgrammableFilter *warpfil;

  /// The internal warp filter's execute function.
  static void WarpHandler(void *obj);
  void DoEffortWarping();
  void UpdateXYBounds();
  void UpdateZBounds();
  void UpdateScaleFactor();

  vtkEffortSource(const vtkEffortSource& other);            // not implemented
  vtkEffortSource& operator=(const vtkEffortSource& other); // not implemented
};


#endif // VTK_EFFORT_SOURCE_H
