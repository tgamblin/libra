#ifndef WT_1D_DIRECT_H
#define WT_1D_DIRECT_H

#include <vector>
#include "wt_1d.h"
#include "filter_bank.h"
#include "cdf97.h"

namespace wavelet {
  
  /// This class provides functions for doing 1d, direct wavelet transforms
  /// on contiguous data.  Also provides internals for filters and an internal 
  /// buffer to store data being transformed.
  ///
  /// This is designd to be inherited privately by something more sophisticated
  /// that can make use of its functions. e.g. wt_direct. which knows about 
  /// boost matrices.
  class wt_1d_direct : public wt_1d {
  public:
    /// Constructs a new direct wavelet transform with the provided filter bank.
    /// Filter defaults to CDF 9/7 Wavelets.
    wt_1d_direct(filter_bank& f = filter::getCDF97());

    /// Destructor
    virtual ~wt_1d_direct();

    /// Forward transform for raw contiguous data.
    virtual void fwt_1d_single(double *data, size_t n);

    /// Inverse transform for raw contiguous data.
    virtual void iwt_1d_single(double *data, size_t n);

  protected:
    /// Filter bank for this transform
    filter_bank& f;

    /// temporary storage for packing values
    std::vector<double> temp;   

    /// Copies x into temp and symmetrically extends edges by filter size.
    ///   e.g. if filter size is 3 and x is:
    ///            1 2 3 4 5 6 7
    ///   temp is filled with:
    ///      4 3 2 1 2 3 4 5 6 7 6 5 4
    /// Params:
    ///      x   input array data
    ///      n   elements to copy from input
    /// stride   stride of data in input
    void sym_extend(double *x, size_t n, size_t stride = 1, bool interleave = false);
  };

} // namespace

#endif // WT_1D_DIRECT_H
