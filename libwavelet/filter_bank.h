#ifndef WT_FILTER_H
#define WT_FILTER_H

#include <cstdlib>

/// \file filter_bank.h
/// This file provides declarations for the filter banks used by
/// the direct wavelet transform.
namespace wavelet {

  /// Set of filters for a wavelet transform.
  struct filter_bank {
    const size_t size;   /// length of filters
    double *const lpf;   /// Forward transform low-pass filter.
    double *const hpf;   /// Forward transform high-pass filter.
    double *const ilpf;  /// Inverse transform low-pass filter.	
    double *const ihpf;  /// Inverse transform high-pass filter.

    filter_bank(size_t D, const double *lpf = NULL, const double *hpf = NULL, 
		          const double *ilpf = NULL, const double *ihpf = NULL);
    ~filter_bank();
  };
  
} //namespaces

#endif //WT_FILTER_H
