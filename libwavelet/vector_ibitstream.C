#include "vector_ibitstream.h"
#include <cassert>
using namespace std;

#include "wt_utils.h"

namespace wavelet {

  vector_ibitstream::vector_ibitstream(const unsigned char *b, size_t size) 
    : buf(b), end(size), pos(0), mask(0x80) {
    assert(size);
  }


  vector_ibitstream::~vector_ibitstream() { }


  size_t vector_ibitstream::get_in_bytes() {
    return bits_to_bytes(total_bits);
  }


  void vector_ibitstream::next_byte() {
    if (mask != 0x80) {
      pos++;
      mask = 0x80;
    }
  }
} // namespace
