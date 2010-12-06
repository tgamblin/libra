#include <iostream>
#include <fstream>

#include "io_utils.h"
#include "matrix_utils.h"
#include "wavelet.h"
using namespace wavelet;
using namespace std;

void usage() {
  cerr << "Usage: dat2mat -o <output_file> <raw_matrix_file>" << endl;
  exit(1);
}


int main(int argc, char **argv) {
  if (argc != 4) {
    usage();
  }

  string output_file;
  string input_file;
  for (size_t i=0; i < argc; i++) {
    if (string(argv[i]) == "-o") {
      i++;
      if (i >= argc) usage();
      output_file = argv[i];
    } else {
      input_file = argv[i];
    }
  }
  if (output_file.empty() || input_file.empty()) {
    usage();
  }
  
  wt_matrix mat;
  read_matrix(input_file.c_str(), mat);
  
  ofstream out(output_file.c_str());
  write_raw_matrix(mat, out);
}
