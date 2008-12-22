#include "filter_bank.h"

#include <cmath>
#include <cstring>
using namespace std;

namespace wavelet { 

  filter_bank::filter_bank(size_t sz, const double *l, const double *h, const double *il, const double *ih) 
    : size(sz), 
      lpf(new double[size]), 
      hpf(new double[size]), 
      ilpf(new double[size]), 
      ihpf(new double[size])
  {
    if (l  != NULL) memcpy( lpf,  l, size * sizeof(double));
    if (h  != NULL) memcpy( hpf,  h, size * sizeof(double));
    if (il != NULL) memcpy(ilpf, il, size * sizeof(double));
    if (ih != NULL) memcpy(ihpf, ih, size * sizeof(double));
  }

  filter_bank::~filter_bank() {
    delete [] lpf;
    delete [] hpf;
    delete [] ilpf;
    delete [] ihpf;
  }

} // namespace
  
