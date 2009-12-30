#ifndef WT_1D_LIFT_H
#define WT_1D_LIFT_H

#include "wt_1d.h"

namespace wavelet {

  /// 1d lifted wavelet transform.  Currently only supports CDF97 wavelets.
  ///
  /// Provides implementation of wt_1d_single routines for wt_1d interface.
  /// 
  /// Based on the 1d version by Gregoire Pau that is available here:
  ///   http://www.ebi.ac.uk/~gpau/misc/dwt97.c
  class wt_1d_lift : public wt_1d {
  public: 
    /// Default Constructor
    wt_1d_lift() { }
    
    /// Destructor
    virtual ~wt_1d_lift() { }

    /// Foward transform for raw contiguous data.
    virtual void fwt_1d_single(double *data, size_t n);

    /// Inverse transform for raw contiguous data.
    virtual void iwt_1d_single(double *data, size_t n);

  protected:
    /// temporary storage for packing
    std::vector<double> temp;
  }; // wt_1d_lift

} // namespace wavelet

#endif // WT_1D_LIFT_H
