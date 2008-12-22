#ifndef LIFT_WT_2D_H
#define LIFT_WT_2D_H

#include <vector>
#include "wavelet.h"
#include "wt_2d.h"

namespace wavelet { 

  /// This is a lifting implementation of the CDF 9/7 wavelet transform.
  /// Matrices passed in must be 
  /// TODO: modify column routines to use groups for speed.
  /// TODO: arbitrarily-sized matrices.
  ///
  /// by Todd Gamblin October 25, 2007.
  ///
  /// Based on the 1d version by Gregoire Pau that is available here:
  ///   http://www.ebi.ac.uk/~gpau/misc/dwt97.c
  class wt_lift : public wt_2d {
  public:
    /// Default Constructor
    wt_lift();
    
    /// Destructor
    virtual ~wt_lift();

    /// Foward transform for raw contiguous data.
    void fwt_1d(double *data, size_t n);

    /// Inverse transform for raw contiguous data.
    void iwt_1d(double *data, size_t n);

    /// Forward wavelet transform for matrix rows.
    virtual void fwt_row(wt_matrix& mat, size_t row, size_t n) {
      fwt_1d(&mat(row, 0), n);
    }

    /// Forward wavelet transform for matrix cols
    virtual void fwt_col(wt_matrix& mat, size_t col, size_t n);


    /// Inverse wavelet transform for matrix rows.
    virtual void iwt_row(wt_matrix& mat, size_t row, size_t n) {
      iwt_1d(&mat(row, 0), n);
    }

    /// Inverse wavelet transform for matrix cols
    virtual void iwt_col(wt_matrix& mat, size_t col, size_t n);

  private:
    /// temporary storage for packing
    std::vector<double> temp;
  };

} // namespace 

#endif //LIFT_WT_2D_H
