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
