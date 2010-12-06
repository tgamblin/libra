#include <iostream>
#include <fstream>

#include "io_utils.h"
#include "matrix_utils.h"
#include "wavelet.h"
using namespace wavelet;
using namespace std;

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "Usage: mat2dat <raw_matrix_file>" << endl;
    exit(1);
  }
  
  const char *filename = argv[1];
  if (!exists(filename)) {
    cerr << "'" << filename << "': No such file." << endl;
    exit(1);
  }
  
  wt_matrix mat;
  ifstream file(filename);
  read_raw_matrix(file, mat);
  
  output(mat);
}
