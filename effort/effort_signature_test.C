
#include <vector>
#include <iostream>
#include <iomanip>

#include "effort_signature.h"
#include "kmedoids.h"
#include "io_utils.h"
#include "wt_lift.h"

using namespace cluster;
using namespace effort;
using namespace wavelet;
using namespace std;



int main(int argc, char **argv) {
  const int size = 64;
  vector<double> data(size);

  cerr << "data size is " << data.size() << endl;

  for (size_t i=0; i < data.size(); i++) {
    data[i] = ((rand()/(double)RAND_MAX)+i+0.4*i*i-0.02*i*i);
  }
  
  for (int l=0; l < log2pow2(size); l++) {
    cerr << "starting iteration " << l << endl;
    cerr << "1 data size is " << data.size() << endl;

    effort_signature sig(data, l);
    cerr << "2 data size is " << data.size() << endl;

    wt_matrix mat(1, size);
    copy(data.begin(), data.end(), &mat(0,0));

    cerr << "3 data size is " << data.size() << endl;

    wt_lift wt;
    for (int i=0; i < l; i++) {
      wt.fwt_row(mat, 0, size >> i);
    }

    cerr << "4 data size is " << data.size() << endl;
    
    if (sig.size() != (size_t)(size >> l)) {
      cerr << "ERROR: sizes do not match:" << endl;
      cerr << "  Found:    " << sig.size() << endl;
      cerr << "  Expected: " << (size >> l) << endl;
      exit(1);
    }

    for (int i=0; i < (size >> l); i++) {
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
    cerr << "done with iteration " << l << endl;
  }

  kmedoids km;
}
