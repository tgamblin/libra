#ifndef WT_1D_H
#define WT_1D_H

#include "wavelet.h"

namespace wavelet {
  
  class wt_1d {
  public:
    wt_1d() { }
    virtual ~wt_1d() { }

    /// Algorithm for forward transform in 1 dimension.  Applies wavelet transform on
    /// lower-frequency bands recursively up to level, or as far as possible if level is -1.
    virtual int fwt_1d(double * data, size_t len, int level = -1);
    
    /// Algorithm for inverse transform in 1 dimension.  Starts at fwt_level and applies 
    /// inverse transform recursively up to iwt_level times.
    /// 
    /// fwt_level:    level of the fwt applied to the matrix (default max possible)
    /// iwt_level:    level of iwt to perform on the matrix. (defaults to fwt_level)
    virtual int iwt_1d(double *data, size_t len, int fwt_level = -1, int iwt_level = -1);

  protected:
    // single-level implementations of the wavelet transform.
    virtual void fwt_1d_single(double *data, size_t len) = 0;
    virtual void iwt_1d_single(double *data, size_t len) = 0;

  }; // wt_1d
  
} // namespace wavelet

#endif // WT_1D_H
