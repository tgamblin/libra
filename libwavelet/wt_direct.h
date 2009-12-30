#ifndef WT_DIRECT_H
#define WT_DIRECT_H

#include "wavelet.h"
#include "filter_bank.h"
#include "cdf97.h"
#include "wt_1d_direct.h"
#include "wt_2d.h"

namespace wavelet { 

  /// This is a non-lifting, symmetrically extended convolution implementation
  /// of the cdf wavelet transform.  This is not as fast as the lifting 
  /// implementation, but the algorithm used is closer to a parallel implementation.
  /// TODO: arbitrarily-sized matrices.
  class wt_direct : public wt_2d, public wt_1d_direct {
  public:
    /// Constructs a new direct wavelet transform with the provided filter bank.
    /// Filter defaults to CDF 9/7 Wavelets.
    wt_direct(filter_bank& f = filter::getCDF97());

    /// Destructor
    virtual ~wt_direct();

    virtual void fwt_row(wt_matrix& mat, size_t row, size_t n) {
      fwt_1d_single(&mat(row, 0), n);
    }
    
    virtual void iwt_row(wt_matrix& mat, size_t row, size_t n) {
      iwt_1d_single(&mat(row, 0), n);
    }
    
    virtual void fwt_col(wt_matrix& mat, size_t col, size_t n);
    virtual void iwt_col(wt_matrix& mat, size_t col, size_t n);
  };


} // namespace 

#endif //WT_DIRECT_H
