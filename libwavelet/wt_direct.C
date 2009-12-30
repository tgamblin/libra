#include "wt_direct.h"

#include <vector>
#include <sstream>
#include <iomanip>
using namespace std;

#include "wavelet.h"
#include "cdf97.h"
#include "matrix_utils.h"

namespace wavelet { 

  // just inits the filter.
  wt_direct::wt_direct(filter_bank& filter) : wt_2d(), wt_1d_direct(filter) { }

  wt_direct::~wt_direct() { } 

  void wt_direct::fwt_col(wt_matrix& mat, size_t col, size_t n) {
    assert(!(n & 1));
    sym_extend(&mat(0, col), n, mat.size2());

    size_t len = n >> 1;
    for (size_t i=0; i < len; i++) {
      mat(i, col) = mat(len+i, col) = 0;

      for (size_t d=0; d < f.size; d++) {
        mat(i, col) += f.lpf[d] * temp[2*i+d];
        mat(len+i, col) += f.hpf[d] * temp[2*i+d+1];
      }
    }
  }
  

  void wt_direct::iwt_col(wt_matrix& mat, size_t col, size_t n) {
    assert(!(n & 1));

    sym_extend(&mat(0, col), n, mat.size2(), true);
    for (size_t i=0; i < n; i++) {
      mat(i,col) = 0.0;
      for (size_t d=0; d < f.size; d++) {
        // this check upsamples the two bands in the input data
        if ((i+d) & 1) mat(i,col) += f.ihpf[d] * temp[i+d];
        else           mat(i,col) += f.ilpf[d] * temp[i+d];
      }
    }
  }


} // namespaces




