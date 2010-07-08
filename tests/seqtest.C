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
#include <iomanip>
#include <stdexcept>
#include <cstring>
using namespace std;

#include "wt_direct.h"
#include "wt_lift.h"
#include "matrix_utils.h"
using wavelet::wt_matrix;
using namespace wavelet;

static const double TOLERANCE = 1.0e-04;


/// This test calculates the MSE between the lifting implementation and 
/// the convolving implementation of the DWT for matrices of various sizes.
int main(int argc, char **argv) {
  wt_1d_direct direct_1d;
  wt_1d_lift lift_1d;

  wt_direct direct;
  wt_lift lift;

  bool pass = true;
  bool verbose = false;
  for (int i=1; i < argc; i++) {
    if (!strcmp(argv[i], "-v")) verbose = true;
  }
  
  if (verbose) cerr << "===== 1 Dimensional Tranform =====" << endl;

  for (size_t c=1; c < 16; c++) {
    size_t cols = 1 << c;
    wt_matrix mat(1, cols);
    
    srand(100);
    for (size_t j=0; j < mat.size2(); j++) {
      mat(0,j) = ((rand()/(double)RAND_MAX)+j+0.4*j-0.02*j*j);
    }
    
    wt_matrix lifted = mat;
    wt_matrix directed = mat;

    direct_1d.fwt_1d(&directed(0,0), cols);
    lift_1d.fwt_1d(&lifted(0,0), cols);
    double fwt_err = nrmse(lifted, directed);
    
    direct.iwt_2d(directed);
    double iwt_err = nrmse(mat, directed);
    
    bool fwt_pass = (fwt_err <= TOLERANCE);
    if (!fwt_pass) pass = false;
    
    bool iwt_pass = (iwt_err <= TOLERANCE);
    if (!iwt_pass) pass = false;
    
    if (verbose) cout << "Normalized RMSE     " << cols << ":  \t" 
                      << setw(16) << fwt_err 
                      << "\t" << (fwt_pass ? "PASS" : "FAIL") 
                      << setw(16) << iwt_err 
                      << "\t" << (iwt_pass ? "PASS" : "FAIL")
                      << endl;
  }

  if (verbose) cerr << endl
                    << "===== 2 Dimensional Tranform =====" << endl;

  for (size_t r=1; r < 8; r++) {
    for (size_t c=1; c < 8; c++) {
      size_t rows = 1 << r;
      size_t cols = 1 << c;

      wt_matrix mat(rows, cols);
      
      srand(100);
      for (size_t i=0; i < mat.size1(); i++) {
        for (size_t j=0; j < mat.size2(); j++) {
          mat(i,j) = ((rand()/(double)RAND_MAX)+i+0.4*i*i-0.02*i*i*j);
        }
      }

      wt_matrix lifted = mat;
      wt_matrix directed = mat;

      direct.fwt_2d(directed);
      lift.fwt_2d(lifted);
      double fwt_err = nrmse(lifted, directed);

      direct.iwt_2d(directed);
      double iwt_err = nrmse(mat, directed);

      bool fwt_pass = (fwt_err <= TOLERANCE);
      if (!fwt_pass) pass = false;

      bool iwt_pass = (iwt_err <= TOLERANCE);
      if (!iwt_pass) pass = false;
      
      if (verbose) cout << "Normalized RMSE " << rows << " x " << cols << ":  \t" 
                        << setw(16) << fwt_err 
                        << "\t" << (fwt_pass ? "PASS" : "FAIL") 
                        << setw(16) << iwt_err 
                        << "\t" << (iwt_pass ? "PASS" : "FAIL")
                        << endl;
    }
  }

  if (verbose) {
    cout << (pass ? "PASSED" : "FAILED") << endl;
  }

  exit(pass ? 0 : 1);
}
