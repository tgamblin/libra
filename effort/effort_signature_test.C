#include <vector>
#include <iostream>
#include <iterator>

#include "effort_signature.h"
#include "io_utils.h"
#include "wt_lift.h"

using namespace effort;
using namespace wavelet;
using namespace std;


int main(int argc, char **argv) {
  // size of data is optional first argument
  int size = 64;
  if (argc > 1) {
    size = strtol(argv[1], NULL, 0);
  }
  if (!isPowerOf2(size)) {
    cerr << "Size must be a power of 2." << endl;
    exit(1);
  }

  // set up some data to make a signature out of
  vector<double> data(size);
  for (size_t i=0; i < data.size(); i++) {
    data[i] = ((rand()/(double)RAND_MAX)+i+0.4*i*i-0.02*i*i);
  }
  
  // make sure transform is done right for all levels we give it
  for (int l=0; l < log2pow2(size); l++) {
    effort_signature sig(data, l);

    wt_matrix mat(1, size);
    copy(data.begin(), data.end(), &mat(0,0));

    wt_lift wt;
    for (int i=0; i < l; i++) {
      wt.fwt_row(mat, 0, size >> i);
    }

    size_t expected_size = (size_t)(size >> l);
    if (sig.size() != expected_size) {
      cerr << "ERROR: sizes do not match:" << endl;
      cerr << "  Found:    " << sig.size() << endl;
      cerr << "  Expected: " << expected_size << endl;
      exit(1);
    }

    for (size_t i=0; i < expected_size; i++) {
      if (sig[i] != mat(0,i)) {
        cerr << "ERROR: incorrect signature transform." << endl;

        cerr << "  Found:    [";
        copy(sig.begin(), sig.end(), ostream_iterator<double>(cerr, " "));
        cerr << "]" << endl;

        cerr << "  Expected: [";
        copy(&mat(0,0), &mat(0,sig.size()), ostream_iterator<double>(cerr, " "));
        cerr << "]" << endl;

        exit(1);
      }
    }
  }
}
