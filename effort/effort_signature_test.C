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
