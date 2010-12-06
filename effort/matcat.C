#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

#include "io_utils.h"
#include "matrix_utils.h"
#include "wavelet.h"
using namespace wavelet;
using namespace std;

void usage() {
  cerr << "Usage: matcat -o output_matrix matrix_file [...]" << endl;
  exit(1);
}

string output_name;

int main(int argc, char **argv) {
  vector<string> mat_names;

  for (size_t i=1; i < argc; i++) {
    if (string(argv[i]) == "-o") {
      i++;
      if (i >= argc) usage();
      output_name = argv[i];
      
    } else if (!exists(argv[i]) && is_directory(argv[i])) {
      cerr << "Error: '" << argv[i] << "': no such file." << endl;
      exit(1);

    } else {
      mat_names.push_back(argv[i]);
    }
  }

  if (argc <= 1 || output_name.empty()) {
    usage();
  }
  
  // first, go through input files and read only the sizes.
  size_t size1 = 0;
  size_t size2 = 0;
  for (size_t i=0; i < mat_names.size(); i++) {  
    ifstream infile(mat_names[i].c_str());
    size_t cur_size1 = read_generic<size_t>(infile);
    size_t cur_size2 = read_generic<size_t>(infile);
    infile.close();

    if (!size2) {
      size2 = cur_size2;
    } else if (cur_size2 != size2) {
      cerr << "Error: matrices must have the same number of columns!" << endl;
      exit(1);
    }
    
    size1 += cur_size1;
  }

  // now write out the input files one by one, but don't bring them all into memory.
  ofstream outfile(output_name.c_str());
  write_generic(outfile, size1);
  write_generic(outfile, size2);

  for (size_t i=0; i < mat_names.size(); i++) {  
    wt_matrix mat;
    ifstream infile(mat_names[i].c_str());
    read_raw_matrix(infile, mat);
    infile.close();

    for (size_t i=0; i < mat.size1(); i++) {
      for (size_t j=0; j < mat.size2(); j++) {
        write_generic(outfile, mat(i,j));
      }
    }
  }
  outfile.close();
}
