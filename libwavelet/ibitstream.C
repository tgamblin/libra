#include "ibitstream.h"

#include <iostream>

using namespace std;

namespace wavelet {

  ibitstream::ibitstream() { }

  ibitstream::~ibitstream() { }

  void ibitstream::read(unsigned char *buffer, size_t size) {
    if (!size) return;

    size_t pos = 0;
    int shift = 7;
    
    buffer[pos] = 0;
    while (pos < size) {
      buffer[pos] |= (get_bit() << shift);
      shift--;
      if (shift < 0) {
	pos++;
	shift = 7;
      }
    }
    
    cerr << "done with read()" << endl;
  }

} // namespace
