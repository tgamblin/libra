#include "buffered_ibitstream.h"
#include <cassert>
using namespace std;

namespace wavelet {

  buffered_ibitstream::buffered_ibitstream(istream& is, size_t bb, size_t bufsize) 
    : in(is), buf(bufsize), pos(bufsize), mask(0x80), end(bufsize), eof(false) { 
    assert(bufsize);
  }

  buffered_ibitstream::~buffered_ibitstream() { }


  void buffered_ibitstream::reload() {
    in.read(reinterpret_cast<char*>(&buf[0]), buf.size());
    end = in.gcount();

    if (end != buf.size()) {
      eof = true;
    }
    pos = 0;
    mask = 0x80;
  }


  void buffered_ibitstream::next_byte() {
    while (mask != 0x80) {
      get_bit();
    }
  }

} // namespace
