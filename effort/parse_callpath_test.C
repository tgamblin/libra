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
#include <set>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iterator>
#include "effort_key.h"
#include "sampler.h"

using namespace effort;
using namespace std;

string remove_spaces(const string& s) {
  string r(s);
  string::iterator new_end = remove(r.begin(), r.end(), ' ');
  r.erase(new_end, r.end());
  return r;
}

static const char *callpath_tests[] = {
  "/g/g21/gamblin2/src/llnl/pub-dd3d.v2.0/dd3d(0x401eb9) : /lib64/libc-2.5.so(0x1d974) : /g/g21/gamblin2/src/llnl/pub-dd3d.v2.0/dd3d(0x401f90) : /g/g21/gamblin2/src/llnl/pub-dd3d.v2.0/dd3d(0x414434) : /g/g21/gamblin2/opt/libra/chaos_4_x86_64_ib/lib/libpmpi-effort.so(0x3c7c) : /usr/local/tools/mvapich-gnu-debug-0.9.9/lib/shared/libmpich.so.1.0(0x657d9) : /usr/local/tools/mvapich-gnu-debug-0.9.9/lib/shared/libmpich.so.1.0(0x67889) : /usr/local/tools/mvapich-gnu-debug-0.9.9/lib/shared/libmpich.so.1.0(0x4a9f1) : /g/g21/gamblin2/opt/libra/chaos_4_x86_64_ib/lib/libpmpi-effort.so(0x351f) => /g/g21/gamblin2/src/llnl/pub-dd3d.v2.0/dd3d(0x401eb9) : /lib64/libc-2.5.so(0x1d974) : /g/g21/gamblin2/src/llnl/pub-dd3d.v2.0/dd3d(0x401f90) : /g/g21/gamblin2/src/llnl/pub-dd3d.v2.0/dd3d(0x414434) : /g/g21/gamblin2/opt/libra/chaos_4_x86_64_ib/lib/libpmpi-effort.so(0x3c7c) : /usr/local/tools/mvapich-gnu-debug-0.9.9/lib/shared/libmpich.so.1.0(0x657d9) : /usr/local/tools/mvapich-gnu-debug-0.9.9/lib/shared/libmpich.so.1.0(0x67889) : /usr/local/tools/mvapich-gnu-debug-0.9.9/lib/shared/libmpich.so.1.0(0x4aa57) : /g/g21/gamblin2/opt/libra/chaos_4_x86_64_ib/lib/libpmpi-effort.so(0x3705)",

  "(0x14d977c):(0x14d9508):(0x1067880):(0x1067c78):(0x107116c):(0x10d99bc):(0x10ed8d4):(0x10b7768):(0x1263790)=>(0x14d977c):(0x14d9508):(0x1067880):(0x1067c78):(0x107116c):(0x10d99bc):(0x10ed8d4):(0x10b8098):(0x1263790)",

  "(0x14d977c) : (0x14d9508) : (0x1067880) : (0x1067c78) : (0x107116c) : (0x10d99bc) : (0x10ed8d4) : (0x10b7768) : (0x1263790) => (0x14d977c) : (0x14d9508) : (0x1067880) : (0x1067c78) : (0x107116c) : (0x10d99bc) : (0x10ed8d4) : (0x10b8098) : (0x1263790)"
};
static const size_t num_tests = sizeof(callpath_tests) / sizeof(const char *);


int main(int argc, char **argv) {
  for (size_t i=0; i < num_tests; i++) {
    const char *callpaths = callpath_tests[i];

    set<effort_key> keys;
    parse_effort_keys(callpaths, inserter(keys, keys.begin()));
    
    ostringstream expected;
    expected << "[time 0 " << callpaths << "]";
    
    ostringstream actual;
    copy(keys.begin(), keys.end(), ostream_iterator<effort_key>(actual));
    
    if (remove_spaces(actual.str()) != remove_spaces(expected.str())) {

      cout << "FAILED at " << i << endl;
      cout << "ACTUAL:" << endl;
      cout << actual.str() << endl;
      cout << "EXPECTED:" << endl;
      cout << expected.str() << endl;
      exit(1);
    }
  }
  cout << "PASSED" << endl;
  exit(0);
}
