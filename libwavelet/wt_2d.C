#include "wt_2d.h"

#include <stdint.h>
#include <limits.h>
#include <iostream>
#include <io_utils.h>
using namespace std;

namespace wavelet {

  int wt_2d::fwt_2d(wt_matrix& mat, int level) {
    if (level < 0) {
      level = (int)log2pow2(std::max(mat.size1(), mat.size2()));
    }
    assert(level <= log2pow2(std::max(mat.size1(), mat.size2())));
  
    size_t rows = mat.size1();
    size_t cols = mat.size2();
    for (int i=0; i < level; i++) {
      if (cols > 1) for (size_t r=0; r < rows; r++) fwt_row(mat, r, cols);
      if (rows > 1) for (size_t c=0; c < cols; c++) fwt_col(mat, c, rows);

      if (rows > 1) rows >>= 1;
      if (cols > 1) cols >>= 1;
    }

    return level;
  }


  int wt_2d::iwt_2d(wt_matrix& mat, int fwt_level, int iwt_level) {
    if (fwt_level < 0) {
      fwt_level = (int)log2pow2(std::max(mat.size1(), mat.size2()));
    }
    assert(fwt_level <= log2pow2(std::max(mat.size1(), mat.size2())));

    if (iwt_level < 0) {
      iwt_level = INT_MAX;
    }
      
    size_t rows, cols;
    int levels = 0;
    for (int i=fwt_level-1; i >= 0 && levels < iwt_level; i--) {
      rows = mat.size1() >> i;
      if (!rows) rows = 1;

      cols = mat.size2() >> i;
      if (!cols) cols = 1;
      
      if (rows > 1) for (size_t c=0; c < cols; c++) iwt_col(mat, c, rows);
      if (cols > 1) for (size_t r=0; r < rows; r++) iwt_row(mat, r, cols);

      levels++;
    }

    return levels;
  }


} // namespace wavelet
	
