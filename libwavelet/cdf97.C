#include "cdf97.h"

#include <iostream>
#include <cstdlib>
#include <cmath>
using namespace std;

namespace wavelet { namespace filter {

  // static data is function-static here to avoid library initialization issues.
  filter_bank& getCDF97() {
    static filter_bank *cdf97 = NULL;

    /// lpf coefficients
    static const double l[5] = {
      0.026748757411,
      -0.016864118443,
      -0.078223266529,
      0.266864118443,
      0.602949018236
    };
    
    /// hpf coefficients
    static const double h[5] = {
      0.0,
      0.045635881557,
      -0.028771763114,
      -0.295635881557,
      0.557543526229
    };
    
    static const double lpf[9] = {
      l[0], l[1], l[2], l[3], l[4], l[3], l[2], l[1], l[0]
    };
    
    static const double hpf[9] = {
      h[0], h[1], h[2], h[3], h[4], h[3], h[2], h[1], h[0]
    };
    
    static const double ilpf[9] = {
      h[0], -h[1], h[2], -h[3], h[4], -h[3], h[2], -h[1], h[0]
    };
    
    static const double ihpf[9] = {
      l[0], -l[1], l[2], -l[3], l[4], -l[3], l[2], -l[1], l[0]
    };
    
    if (!cdf97) {
      const int size = 9;
      cdf97 = new filter_bank(size, lpf, hpf, ilpf, ihpf);
      
      // apply some scaling here to get this to agree with lifting implementation
      for (int i=0; i < size; i++) {
	cdf97->lpf[i]  *= sqrt(2.0);
	cdf97->hpf[i]  *= sqrt(2.0);
	// output of transform is decimated by 2, so need to multiply iwt filters by 2
	cdf97->ilpf[i] *= 2.0/sqrt(2.0);
	cdf97->ihpf[i] *= 2.0/sqrt(2.0);
      }
    }
    return *cdf97;
  }
  
}} // namespace
  
