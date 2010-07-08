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
#include <sys/time.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#include "wt_utils.h"
using namespace wavelet;


int main(int argc, char **argv) {
  struct timeval time;
  gettimeofday(&time, 0);
  srand(time.tv_sec * 1000000 + time.tv_usec);

  bool pass = true;
  bool verbose = false;
  for (int i=1; i < argc; i++) {
    if (!strcmp(argv[i], "-v")) verbose = true;
  }
  
  long max_bits = 70;
  for (long src_bits=0; src_bits < max_bits; src_bits++) {
    for (long dest_bits=0; dest_bits < max_bits; dest_bits++) {
      for (long src_offset=0; src_offset < src_bits; src_offset++) {
        long src_bytes  = bits_to_bytes(src_bits + src_offset);
        long dest_bytes = bits_to_bytes(dest_bits);
	
        long total_bits = src_bits + dest_bits;
        long total_bytes = bits_to_bytes(total_bits);
	
        if (!src_bytes) src_bytes = 1;
        if (!dest_bytes) dest_bytes = 1;
        if (!total_bytes) total_bytes = 1;
	
        unsigned char *src = new unsigned char[src_bytes];
        unsigned char *dest = new unsigned char[total_bytes];
	
        for (int i=0; i < src_bytes; i++) {
          src[i] = (unsigned char)(rand() / (double)RAND_MAX * 256.0);
        }
	
        for (int i=0; i < dest_bytes; i++) {
          dest[i] = (unsigned char)(rand() / (double)RAND_MAX * 256.0);
        }
	

        ostringstream dest_out;	
        print_bits(dest_out, dest_bits, dest);
        string dest_str = dest_out.str();

        ostringstream src_out;
        print_bits(src_out, src_bits + src_offset, src);
        string src_str = src_out.str().substr(src_offset, src_bits);
	
        insert_bits(dest, src, src_bits, dest_bits, src_offset);
	
        ostringstream actual_out;
        print_bits(actual_out, src_bits + dest_bits, dest);
	
        string expected = (dest_str + src_str);
        string actual = actual_out.str();
        if (actual != expected) {
          pass = false;
          if (verbose) {
            cout << "FAIL:" << endl;
            cout << "  src_bits:   " << src_bits << endl;
            cout << "  dest_bits:  " << dest_bits << endl;
            cout << "  src_offset: " << src_offset << endl;
            cout << "  src:        " << src_out.str() << endl;
            cout << "  dest:       " << dest_str << endl;
            cout << "  expected:   " << expected << endl;
            cout << "  actual:     " << actual << endl;
          }
        }
	
        delete [] src;
        delete [] dest;
      }
    }
  }

  if (verbose) {
    cout << (pass ? "PASSED" : "FAILED") << endl;
  }

  exit(pass ? 0 : 1);
}


