/////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010, Lawrence Livermore National Security, LLC.  
// Produced at the Lawrence Livermore National Laboratory  
// Written by Todd Gamblin, tgamblin@llnl.gov.
// LLNL-CODE-417602
// All rights reserved.  
// 
// This file is part of Libra. For details, see http://github.com/tgamblin/libra.
// Please also read the LICENSE file for further information.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the disclaimer below.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the disclaimer (as noted below) in the documentation and/or other materials
//    provided with the distribution.
//  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
//    or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
/////////////////////////////////////////////////////////////////////////////////////////////////
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

