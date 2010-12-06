#include <fstream>
#include <vector>
#include "io_utils.h"
#include "wavelet.h"
using namespace wavelet;
using namespace std;

void init_matrix(wt_matrix& mat, double& val, size_t& size1) {
  for (size_t i=0; i < mat.size1(); i++) {
    for (size_t j=0; j < mat.size2(); j++) {
      mat(i,j) = val;
      val += 1;
    }
  }
  size1 += mat.size1();
}

void write_matrix(const wt_matrix& mat, const char *filename) {
  ofstream file(filename);
  write_raw_matrix(mat, file);
}


int main(int argc, char **argv) {
  wt_matrix mat1(4,  8);
  wt_matrix mat2(10, 8);
  wt_matrix mat3(9,  8);
  wt_matrix mat4(2,  8);
  
  double val = 0;
  size_t size1 = 0;
  init_matrix(mat1, val, size1);
  write_matrix(mat1, "mat1");

  init_matrix(mat2, val, size1);
  write_matrix(mat2, "mat2");

  init_matrix(mat3, val, size1);
  write_matrix(mat3, "mat3");

  init_matrix(mat4, val, size1);
  write_matrix(mat4, "mat4");
  
  system("matcat mat1 mat2 mat3 mat4 -o mat");

  wt_matrix full;
  ifstream infile("mat");
  read_raw_matrix(infile, full);
  infile.close();

  if (full.size1() != size1) {
    cerr << "FAILED: size1 does not agree: " << full.size1() << ", expected " << size1 << endl;
    exit(1);
  }
  
  if (full.size2() != mat1.size2()) {
    cerr << "FAILED: size2 does not agree: " << full.size2() << ", expected " << mat1.size2() << endl;
    exit(1);
  }

  val = 0;
  for (size_t i=0; i < full.size1(); i++) {
    for (size_t j=0; j < full.size2(); j++) {
      if (full(i,j) != val) {
        cerr << "FAILED: value does not agree: " << full(i,j)
             << ", expected " << val << " at (" <<  i << "," << j << ")" 
             << endl;
        cerr << "leaving temporary files in mat1, mat2, mat3, mat4, and mat." << endl;
        exit(1);
      }
      val += 1;
    }
  }
  
  //system("rm -f mat1 mat2 mat3 mat4 mat");
  cerr << "PASSED" << endl;
  exit(0);
}
