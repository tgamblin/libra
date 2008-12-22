#ifndef WT_IO_UTILS_H
#define WT_IO_UTILS_H

#include <stdint.h>
#include <cstdlib>
#include <iostream>

namespace wavelet {

  // Variable-length read and write routines for unsigned numbers.
  size_t vl_write(std::ostream& out, unsigned long long size);
  unsigned long long vl_read(std::istream& in);


  /// Endian-agnostic write for integer types. This doesn't compress
  /// like vl_write, but it handles signs.
  template<class T>
  size_t write_generic(std::ostream& out, T num) {
    for (size_t i=0; i < sizeof(T); i++) {
      unsigned char lo_bits = (num & 0xFF);
      out.write((char*)&lo_bits, 1);
      num >>= 8;
    }
    return sizeof(T);
  }


  /// Endian-agnostic read for integer types. This doesn't compress
  /// like vl_write, but it handles signs.
  template<class T>
  T read_generic(std::istream& in) {
    T num = 0;
    for (size_t i=0; i < sizeof(T); i++) {
      unsigned char byte;
      in.read((char*)&byte, 1);
      num |= ((T)byte) << (i<<3);
    }
    return num;
  }
  

  /// Test for integral types to make sure they're powers of two.
  template <class T>
  bool isPowerOf2(T num) { return !(num & (num-1)); }


  /// Returns next highest power of two for an integral type (up to 64 bits)
  inline int nextPowerOf2(uint32_t num) {
    num--;
    num |= (num >> 1);  // these fill with ones.
    num |= (num >> 2);
    num |= (num >> 4);
    num |= (num >> 8);
    num |= (num >> 16);
    num++;
    return num;
  }

  inline int nextPowerOf2(uint64_t num) {
    num--;
    num |= (num >> 1);  // these fill with ones.
    num |= (num >> 2);
    num |= (num >> 4);
    num |= (num >> 8);
    num |= (num >> 16);
    num |= (num >> 32);
    num++;
    return num;
  }

  /// Takes the log base 2 of a power of 2, returns a char.
  /// Returns -1 if 0 is passed in.
  signed char log2pow2(unsigned long long powerOf2);

} //namespace

#endif // WT_IO_UTILS_H
