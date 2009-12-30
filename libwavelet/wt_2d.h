#ifndef WT_2D_H
#define WT_2D_H

#include "wavelet.h"

namespace wavelet {

  /// Abstract superclass for transform classes.  Provides methods for 2d transforms.
  /// This allows classes that provide 1d transform implementations to easily export
  /// a 2d API as well.
  class wt_2d {
  public:
    /// Constructor
    wt_2d() { }

    /// Destructor
    virtual ~wt_2d() { }

    /// Algorithm for forward transform in 2 dimensions.  Applies alternating 1d
    /// transforms for rows and columns.  Returns the number of foward transforms
    /// (levels) that were applied.
    /// completed.
    /// mat:          matrix to perform the forward transform on
    /// level:        level of fwt to apply to them matrix.
    virtual int fwt_2d(wt_matrix& mat, int level = -1);
    
    
    /// Algorithm for inverse transform in 2 dimensions.  Applies alternating 1d
    /// inverse transforms for rows and columns. Returns the number of iwt transforms
    /// (levels) applied to the matrix.
    /// Parameters:
    /// mat:          matrix to perform the inverse transform on
    /// fwt_level:    level of the fwt applied to the matrix (default max possible)
    /// iwt_level:    level of iwt to perform on the matrix. (defaults to fwt_level)
    virtual int iwt_2d(wt_matrix& mat, int fwt_level = -1, int iwt_level = -1);

    /// Forward wavelet transform for matrix rows.
    /// mat: a boost matrix containing the data to be transformed
    /// row: the row to transform
    /// n:   length of the row, starting at 0, to transform
    virtual void fwt_row(wt_matrix& mat, size_t row, size_t n) = 0;

    /// Forward wavelet transform for matrix columns.
    /// mat: a boost matrix containing the data to be transformed
    /// col: the column to transform
    /// n:   length of the column, starting at 0, to transform
    virtual void fwt_col(wt_matrix& mat, size_t col, size_t n) = 0;

    /// Inverse transform for matrix rows.
    /// mat: a boost matrix containing the data to be transformed
    /// row: the row to transform
    /// n:   length of the row, starting at 0, to transform
    virtual void iwt_row(wt_matrix& mat, size_t row, size_t n) = 0;

    /// Inverse transform for matrix columns
    /// mat: a boost matrix containing the data to be transformed
    /// col: the column to transform
    /// n:   length of the column, starting at 0, to transform
    virtual void iwt_col(wt_matrix& mat, size_t col, size_t n) = 0;
  };


} // namespace

#endif // WT_2D_H



