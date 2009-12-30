#include "wt_1d.h"

#include "io_utils.h"
using namespace std;

namespace wavelet {

  int wt_1d::fwt_1d(double * data, size_t len, int level) {
    if (level < 0) {
      level = (int)log2pow2(len);
    }
    assert(level <= log2pow2(len));

    size_t cur_len = len;
    for (int i=0; i < level; i++) {
      fwt_1d_single(data, cur_len);
      cur_len >>= 1;
    }

    return level;    
  }
  

  int wt_1d::iwt_1d(double *data, size_t len, int fwt_level, int iwt_level) {
    if (fwt_level < 0) {
      fwt_level = (int)log2pow2(len);
    }
    assert(fwt_level <= log2pow2(len));

    if (iwt_level < 0) {
      iwt_level = INT_MAX;
    }
      
    int levels = 0;
    for (int i=fwt_level-1; i >= 0 && levels < iwt_level; i--) {
      size_t cur_len = len >> i;
      iwt_1d_single(data, cur_len);
      levels++;
    }

    return levels;    
  }

} // namespace wavelet
