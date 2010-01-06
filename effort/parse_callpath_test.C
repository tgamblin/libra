#include <set>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iterator>
#include "effort_key.h"
#include "sampler.h"

using namespace effort;
using namespace std;

static const char *callpaths = "/g/g21/gamblin2/src/llnl/pub-dd3d.v2.0/dd3d(0x401eb9) : /lib64/libc-2.5.so(0x1d974) : /g/g21/gamblin2/src/llnl/pub-dd3d.v2.0/dd3d(0x401f90) : /g/g21/gamblin2/src/llnl/pub-dd3d.v2.0/dd3d(0x414434) : /g/g21/gamblin2/opt/libra/chaos_4_x86_64_ib/lib/libpmpi-effort.so(0x3c7c) : /usr/local/tools/mvapich-gnu-debug-0.9.9/lib/shared/libmpich.so.1.0(0x657d9) : /usr/local/tools/mvapich-gnu-debug-0.9.9/lib/shared/libmpich.so.1.0(0x67889) : /usr/local/tools/mvapich-gnu-debug-0.9.9/lib/shared/libmpich.so.1.0(0x4a9f1) : /g/g21/gamblin2/opt/libra/chaos_4_x86_64_ib/lib/libpmpi-effort.so(0x351f) => /g/g21/gamblin2/src/llnl/pub-dd3d.v2.0/dd3d(0x401eb9) : /lib64/libc-2.5.so(0x1d974) : /g/g21/gamblin2/src/llnl/pub-dd3d.v2.0/dd3d(0x401f90) : /g/g21/gamblin2/src/llnl/pub-dd3d.v2.0/dd3d(0x414434) : /g/g21/gamblin2/opt/libra/chaos_4_x86_64_ib/lib/libpmpi-effort.so(0x3c7c) : /usr/local/tools/mvapich-gnu-debug-0.9.9/lib/shared/libmpich.so.1.0(0x657d9) : /usr/local/tools/mvapich-gnu-debug-0.9.9/lib/shared/libmpich.so.1.0(0x67889) : /usr/local/tools/mvapich-gnu-debug-0.9.9/lib/shared/libmpich.so.1.0(0x4aa57) : /g/g21/gamblin2/opt/libra/chaos_4_x86_64_ib/lib/libpmpi-effort.so(0x3705)";

int main(int argc, char **argv) {
  set<effort_key> keys;
  parse_effort_keys(callpaths, inserter(keys, keys.begin()));

  ostringstream expected;
  expected << "[time 0 " << callpaths << "]";
  
  ostringstream actual;
  copy(keys.begin(), keys.end(), ostream_iterator<effort_key>(actual));
  
  if (actual.str() != expected.str()) {
    cout << "FAILED" << endl;
    cout << "ACTUAL:" << endl;
    cout << actual.str() << endl;
    cout << "EXPECTED:" << endl;
    cout << expected.str() << endl;
    exit(1);
  } else {
    cout << "PASSED" << endl;
    exit(0);
  }
}
