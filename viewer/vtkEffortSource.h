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
