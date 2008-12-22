#include "obitstream.h"

#include <iostream>
using namespace std;

#include "wt_utils.h"

namespace wavelet {

  obitstream::obitstream() { }


  obitstream::~obitstream() { }
  

  void obitstream::put_bit(bool bit) {
    if (bit) {
      put_one();
    } else {
      put_zero();
    }
  }


  void obitstream::write(const unsigned char *buf, size_t bytes) {
    write_bits(buf, bytes << 3);
  }


  void obitstream::write_bits(const unsigned char *src, size_t src_bits, size_t src_offset) {
    size_t pos = (src_offset >> 3);
    unsigned char mask = (0x80 >> (src_offset & 0x7l));

    for (size_t i=0; i < src_bits; i++) {
      if (src[pos] & mask) {
	put_one();
      } else {
	put_zero();
      }
      mask >>= 1;

      if (mask == 0) {
	pos++;
	mask = 0x80;
      }
    }
  }

  

} // namespace
