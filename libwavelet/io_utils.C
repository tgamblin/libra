#include "io_utils.h"
using namespace std;

#include <cassert>

namespace wavelet {

  size_t vl_write(ostream& out, unsigned long long size) {
    int out_bytes = 0;

    do {
      unsigned long long out_byte = (long long)(size & 0x7Fll);
      if ((size >>= 7) > 0) out_byte |= 0x80;

      char oc = (char)out_byte;
      out.write(&oc, 1);

      if (!out.good()) {
        cerr << "Error: can't write to file." << endl;
        exit(1);
      }
      out_bytes++;
    } while (size);

    return out_bytes;
  }
  
  
  unsigned long long vl_read(istream& in) {
    unsigned shift = 0;
    unsigned long long long_bytes = 0;

    char ichar;
    unsigned long long file_byte = 0;

    // read variable-length header with number of code bytes
    do {
      in.read(&ichar, 1);
      file_byte = (file_byte & ~0xFF) | ichar;
      if (!in.good()) return 0;

      long_bytes |= (long long)(file_byte & 0x7Fll) << shift;
      shift += 7;
    } while (file_byte & 0x80);

    return long_bytes;
  }


  signed char log2pow2(unsigned long long powerOf2) {
    // make sure it's a power of 2.
    assert(isPowerOf2(powerOf2));
    
    signed char n = -1;
    while (powerOf2 > 0) {
      powerOf2 >>= 1;
      n++;
    }

    return n;
  }

} //namespace
