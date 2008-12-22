#include <sstream>
#include <cstring>
#include <iostream>
using namespace std;

#include "io_utils.h"
using namespace wavelet;

/// This is a test of the variable-length write routines used
/// to write compressed header files.


int main(int argc, char **argv) {
  ostringstream os;
  istringstream is;

  bool pass = true;
  bool verbose = false;
  for (int i=1; i < argc; i++) {
    if (!strcmp(argv[i], "-v")) verbose = true;
  }
  

  for (size_t i=0; i < 1048576; i+=17) {
    os.str("");
    vl_write(os, i);

    is.str(os.str());
    size_t j = vl_read(is);

    if (i != j) {
      if (verbose) {
        cout << "expected " << i << " but got " << j << endl;
      }
      pass = false;
    }
  }
  
  if (verbose) {
    cout << (pass ? "PASSED" : "FAILED") << endl;
  }

  exit(pass ? 0 : 1);
}
