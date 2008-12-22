#ifndef WAVELET_H
#define WAVELET_H

#include "matrix_utils.h"
#include <algorithm>

///\file wavelet.h
/// This header includes declarations for types used in all wavelet transforms
namespace wavelet { 

  /// Matrix type used for all computations here.
  typedef boost::numeric::ublas::matrix<double> wt_matrix;

} // namespaces

#endif // WAVELET_H
