#include "ac_obitstream.h"
#include "wt_utils.h"
#include "io_utils.h"
using namespace std;

#include "arithmetic_codec.h"
#include "byte_budget_exception.h"

#define CHECK_BYTES \
do { \
    if (byte_budget && (encoder->get_code_bytes() + get_out_bytes()) >= byte_budget) { \
      throw byte_budget_exception(); \
    } \
} while (0) 


namespace wavelet {

  ac_obitstream::ac_obitstream(ostream& o, size_t bb, size_t bs) 
    : out(o), bufsize(bs), byte_budget(bb), cur_bits(0), 
      bit_model(new Adaptive_Bit_Model()), encoder(new Arithmetic_Codec(2*bs)), 
      bits(0), coded_bytes(0)
  { 
    encoder->start_encoder();
  }
  
  
  ac_obitstream::~ac_obitstream() { 
    flush();
  }
  
  
  void ac_obitstream::put_zero() {
    CHECK_BYTES;

    encoder->encode((unsigned)0, *bit_model);
    cur_bits++;
    bits++;
    if ((cur_bits >> 3) == bufsize) {
      flush();
    }
  }


  void ac_obitstream::put_one() {
    CHECK_BYTES;

    encoder->encode((unsigned)1, *bit_model);
    cur_bits++;
    bits++;
    if ((cur_bits >> 3) == bufsize) {
      flush();
    }
  }
  

  void ac_obitstream::next_byte() {
    while (bits & 0x7l) {
      put_zero();
    }
  }

  
  void ac_obitstream::flush() {
    if (cur_bits) {
      long cur_bytes = bits_to_bytes(cur_bits);
      long enc_bytes = encoder->stop_encoder();

      coded_bytes += vl_write(out, cur_bytes);
      coded_bytes += vl_write(out, enc_bytes);
      out.write((char*)encoder->buffer(), enc_bytes);
      coded_bytes += enc_bytes;

      cur_bits = 0;
      encoder->start_encoder();
    }
  }


  size_t ac_obitstream::get_in_bits() {
    return bits;
  }
  

  size_t ac_obitstream::get_in_bytes() {
    return bits_to_bytes(bits);
  }
  

  size_t ac_obitstream::get_out_bits() {
    return (coded_bytes << 3);
  }


  size_t ac_obitstream::get_out_bytes() {
    return coded_bytes;
  }

  void ac_obitstream::write_bits(const unsigned char *src, size_t src_bits, size_t src_offset) {
    size_t pos = (src_offset >> 3);
    unsigned char mask = (0x80 >> (src_offset & 0x7l));

    for (size_t i=0; i < src_bits; i++) {
      unsigned bit = (src[pos] & mask) ? 1 : 0;
      encoder->encode(bit, *bit_model);
      cur_bits++;
      bits++;
      if ((cur_bits >> 3) == bufsize) {
	flush();
      }

      mask >>= 1;

      if (mask == 0) {
	pos++;
	mask = 0x80;
      }
    }
  }


} //namespace
