#include "wavelet_ssim.h"
#include "matrix_utils.h"
#include "wt_direct.h"
using namespace wavelet;

#include <iostream>
#include <cmath>
using namespace std;


int main(int argc, char **argv) {
  wt_direct wt;
  const size_t size = 32;

  wt_matrix m1(size,size);
  for (size_t i=0; i < m1.size1(); i++) {
    for (size_t j=0; j < m1.size2(); j++) {
      m1(i,j) = i;
    }
  }
  
  wt_matrix m2(size,size);
  for (size_t i=0; i < m2.size1(); i++) {
    for (size_t j=0; j < m2.size2(); j++) {
      m2(i,j) = i*8;
    }
  }

  int level = wt.fwt_2d(m1);
  cerr << "actual level is " << level << endl;
  wt.fwt_2d(m2);

  cout << sqrt(2*(1-wssim(m1, m2))) << endl;
}
