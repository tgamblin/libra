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
#include <cstdlib>
#include <cstdio>
#include <fstream>
using namespace std;

#include "wavelet.h"
#include "wt_lift.h"
#include "wt_direct.h"
#include "matrix_utils.h"
#include "ezw_encoder.h"
#include "ezw_decoder.h"
using wavelet::wt_matrix;
using namespace wavelet;

static const char *PROGNAME = "vary_passes";
static const char *EZW_FILE = "vary_passes.out";

int main(int argc, char **argv) {
  char *filename;
  long long scale = 1;
  char *err;
  switch (argc) {
  case 3:
    scale = strtoll(argv[2], &err, 10);
    if (err && *err) {
      cerr << "Error: invalid scale: " << argv[2] << endl;
      exit(1);
    }

  case 2:
    filename = argv[1];
    break;

  default:
    cerr << "Usage: " << PROGNAME << " file [scale]" << endl;
    exit(1);
  }

  wt_lift wt;

  printf("%8s    %8s    %8s    %8s    %8s    %10s    %8s\n", 
         "PASS", "%TOTAL", "%EZW", "NRMSE", "PSNR", "RATE", "BYTES");

  for (int limit=1; limit <= 60; limit++) {
    ezw_encoder encoder;
    encoder.set_pass_limit(limit);

    encoder.set_scale(scale);
    ezw_decoder decoder;
  
    wt_matrix mat;
    read_matrix(filename, mat);

    // transform values
    wt_matrix trans = mat;
    int level = wt.fwt_2d(trans);

    // ezw-encode the transformed matrix
    ofstream encoded_out(EZW_FILE);
    long bytes = encoder.encode(trans, encoded_out, level);
    encoded_out.close();

    ifstream file(EZW_FILE);
    ezw_header header;
    ezw_header::read_in(file, header);
    quantized_t threshold = header.threshold;
  
    int passes = 0;
    while (threshold) {
      threshold >>= 1;
      passes++;
    }

    const long size = mat.size1() * mat.size2() * sizeof(double);

    // decode the ezw-coded file.
    ifstream encoded_in(EZW_FILE);
    wt_matrix unezw;
    decoder.decode(encoded_in, unezw, -1, &header);
    size_t read = decoder.get_bytes_read();

    // inverse-transform the decoded data and get error
    wt.iwt_2d(unezw, level);
    double err = nrmse(mat, unezw);
    double psnr_val = psnr(mat, unezw);

    double compression = read/(double)size;
    double ezw_fraction = read/bytes;
    double rate = size / read;
    
    printf("%8d    %8.4f    %8.4f    %8.4f    %8.4f    %8.2f:1    %8zd\n", 
           limit, compression, ezw_fraction, err, psnr_val, rate, read);
  }
}
