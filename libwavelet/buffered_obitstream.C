#include "buffered_obitstream.h"
#include "wt_utils.h"

using namespace std;

namespace wavelet {

  buffered_obitstream::buffered_obitstream(ostream& o, size_t byte_budget, size_t bufsize) 
    : buf(bufsize), pos(0), mask(0x80), bits(0), out(o)
  { 
    buf[0] = 0;
  }


  buffered_obitstream::~buffered_obitstream() {
    flush();
  }
  
  
  void buffered_obitstream::put_zero() {
    mask >>= 1;
    bits++;
    if (mask == 0) {
      pos++;
      mask = 0x80;
      
      if (pos == buf.size()) {
	my_flush();
      } else {
	buf[pos] = 0;
      }
    }
  }


  void buffered_obitstream::put_one() {
    buf[pos] |= mask;
    put_zero();
  }
  

  void buffered_obitstream::write_bits(const unsigned char *src, size_t src_bits, size_t src_offset) {
    size_t offset = (pos << 3);
    for (unsigned m = mask; m != 0x80; m <<= 1) offset++;
    size_t max_bits = (buf.size() << 3);

    while (src_bits) {
      size_t to_write = min(max_bits - offset, src_bits);
      insert_bits(&buf[0], src, to_write, offset, src_offset);

      src_bits -= to_write;
      src_offset += to_write;
      bits += to_write;
      offset += to_write;

      pos = (offset >> 3);                      // reset buffer index
      mask = (0x80 >> (offset & 0x7l));         // reset bit mask for last bit.
      buf[pos] &= ~(0xFF >> (offset & 0x7l));   // zero out remaining bits in trailing byte.

      if (offset == max_bits) {
	my_flush();
	offset = 0;
      }
    }
  }

  
  void buffered_obitstream::my_flush() {
    size_t buf_bits = (pos << 3) + (mask == 0x80 ? 0 : 1);
    if (buf_bits) {
      out.write(reinterpret_cast<char*>(&buf[0]), bits_to_bytes(buf_bits));
      pos = 0;
      buf[pos] = 0;
      mask = 0x80;
    }
  }


  void buffered_obitstream::flush() {
    my_flush();
    out.flush();
  }


  size_t buffered_obitstream::get_in_bits() {
    return bits;
  }


  size_t buffered_obitstream::get_in_bytes() {
    return bits_to_bytes(bits);
  }


  size_t buffered_obitstream::get_out_bits() {
    return bits;
  }


  size_t buffered_obitstream::get_out_bytes() {
    return bits_to_bytes(bits);
  }


  void buffered_obitstream::next_byte() {
    while (mask != 0x80) {
      put_zero();
    }
  }

} // namespace 
