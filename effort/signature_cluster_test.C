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
#include <vector>
#include <iostream>
#include <iterator>

#include "effort_signature.h"
#include "io_utils.h"
#include "wavelet.h"
#include "kmedoids.h"
#include "matrix_utils.h"
#include "bic.h"

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
    cout << "BIC: " << bic(km, matrix_distance(distance), data.size2()) << endl;
  }
}
