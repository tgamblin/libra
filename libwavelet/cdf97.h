#ifndef WT_CDF97_FILTER_H
#define WT_CDF97_FILTER_H

#include "filter_bank.h"

/// \file cdf97.h
/// This file provides declarations for the filters used by
/// Cohen-Daubechies-Feauveau 9/7 wavelets. We include both the
/// standard filter and the lifting filter here.
namespace wavelet { namespace filter {
  
  /// Returns a singleton instance of the CDF97 filter bank.
  filter_bank& getCDF97();

}} //namespaces

#endif //WT_CDF97_FILTER_H
