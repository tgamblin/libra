#include <vector>
#include <iostream>
#include <iterator>

#include "effort_signature.h"
#include "io_utils.h"
#include "wavelet.h"
#include "kmedoids.h"
#include "matrix_utils.h"

using namespace effort;
using namespace cluster;
using namespace std;
using boost::numeric::ublas::matrix;


int main(int argc, char **argv) {
  // size of data is optional first argument
  int arg = 0;

  arg++;
  int level = -1;
  if (argc > arg) level = strtol(argv[arg], NULL, 0);

  arg++;
  size_t num_sigs = 16;
  if (argc > arg) num_sigs = strtol(argv[arg], NULL, 0);

  arg++;
  size_t trace_size = 64;
  if (argc > arg) trace_size = strtol(argv[arg], NULL, 0);
  if (!wavelet::isPowerOf2(trace_size)) {
    cerr << "Trace size must be a power of 2." << endl;
    exit(1);
  }
  
  
  // initialize data with interesting functions
  matrix<double> data(num_sigs, trace_size);

  srand(142859287);
  double noise = 1.0;
  for (size_t r=0; r < data.size1(); r++) {
    size_t type = r % 3;

    for (size_t c=0; c < data.size2(); c++) {
      switch (type) {
      case 0:
        data(r,c) = 10 * sin(c/5.0);
        break;
      case 1:
        data(r,c) = sin(c/5.0);
        break;
      case 2:
        data(r,c) = 10 * sin(c/5.0)*sin(c/5.0) * cos(c);
        break;
      }
      
      // add in noise so that things aren't *exactly* the same.
      double e = (noise * (rand()/(double)RAND_MAX));
      data(r,c) += e;
    }
  }

  vector<effort_signature> sigs(data.size1());
  for (size_t i=0; i < data.size1(); i++) {
    sigs[i] = effort_signature(&data(i,0), data.size2(), level);
  }
  
  dissimilarity_matrix distance;
  build_dissimilarity_matrix(sigs, sig_euclidean_distance(), distance);

  for (size_t k=1; k <= 10; k++) {
    kmedoids km;
    km.set_epsilon(0);
    km.pam(distance, k);
    cout << km << endl;
  }
}
