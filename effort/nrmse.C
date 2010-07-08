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
#include <iostream>
#include <fstream>
using namespace std;

#include "wavelet.h"
#include "wt_direct.h"
#include "ezw.h"
#include "ezw_decoder.h"
#include "matrix_utils.h"
using namespace wavelet;

#include "effort_key.h"
using namespace effort;


void usage() {
  cerr << "Usage: nrmse m1 [m2]" << endl;
  cerr << "  Calculates normalized root mean squared error between m1 and m2." << endl;
  cerr << "  Also outputs PSNR and similarity (symmetric NRMSE)." << endl;
  cerr << "  If m2 is not provided, tries to find it in the exact/ subfolder, if it exists." << endl;
  exit(1);
}


int main(int argc, char **argv) {
  if (argc < 2 || argc > 3) {
    usage();
  }

  string compressed_filename(argv[1]);

  wavelet::wt_matrix exact;

  if (argc > 2) {
    string exact_filename(argv[2]);
    ifstream exact_file(exact_filename.c_str());

    effort_key key;
    ezw_decoder decoder;
    wt_direct wt;

    effort_key::read_in(exact_file, key);
    int level = decoder.decode(exact_file, exact);
    wt.iwt_2d(exact, level);
    
  } else {
    string metric;
    int type, number;
    if (!parse_filename(compressed_filename, &metric, &type, &number)) {
      cerr << "Can't make sense of filename: " << compressed_filename << endl;
    }

    ostringstream exact_str;
    exact_str << "exact/exact-" << metric << "-" << type << "-" << number;

    string exact_filename = exact_str.str();
    if (!read_matrix(exact_filename.c_str(), exact)) {
      cerr << "Couldn't open file: '" << exact_filename << "'" << endl;
      exit(1);
    }
 }


  wavelet::wt_matrix reconstruction;
  ifstream comp_file(compressed_filename.c_str());
  if (comp_file.fail()) {
    cerr << "Couldn't open file: '" << compressed_filename << "'" << endl;
    exit(1);
  }

  effort_key key;
  ezw_decoder decoder;
  wt_direct wt;

  effort_key::read_in(comp_file, key);
  int level = decoder.decode(comp_file, reconstruction);
  wt.iwt_2d(reconstruction, level);

  // output error to the metadata file if we're verifying.
  cout << "NRMSE:\t" << nrmse(exact, reconstruction) << endl;
  cout << "PSNR:\t" << psnr(exact, reconstruction) << endl;
  cout << "SIMIL:\t" << similarity(exact, reconstruction) << endl;
}
