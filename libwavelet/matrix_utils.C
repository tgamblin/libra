#include "matrix_utils.h"

#include <fstream>
#include <iostream>
using namespace std;

bool read_matrix(const char *filename, boost::numeric::ublas::matrix<double>& mat) {
  ifstream file(filename);
  if (file.fail()) return false;

  int cols = 0;
  int rows = 0;
  char c;

  // count cols on first line (assume same for following lines)
  file.get(c);
  while (file.good() && c != '\n') {
    while (file.good() && c != '\n' && c == ' ') {
      file.get(c);
    }
    if (c != '\n') {
      cols++;
    }
    while (file.good() && c != '\n' && c != ' ') {
      file.get(c);
    }
  }

  if (!file.good()) return false;

  // count remaining lines
  while (file.good()) {
    if (c == '\n') rows++;
    file.get(c);
  }
  
  file.close();  // close
  file.clear();  // reset flags
  file.open(filename);   // open at beginning.
  
  mat.resize(rows, cols);
  string line;
  for (int row=0; row < rows; row++) {
    getline(file, line);
    int pos = 0;

    for (int col=0; col < cols; col++) {
      int start = line.find_first_not_of(" ", pos);
      int end = line.find_first_of(" ", start);
      mat(row,col) = strtod(&line[start], NULL);
      pos = end;
    }
  }

  return true;
}


bool isDivisibleBy2(size_t n, int level) {
  while (level != 0) {
    if (n & ((size_t)0x1)) {
      return false;
    }
    n >>= 1;
    level--;
  }

  return true; 
}

