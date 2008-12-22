#include "ac_ibitstream.h"

#include "arithmetic_codec.h"
#include "byte_budget_exception.h"
#include "wt_utils.h"
#include "io_utils.h"

using namespace std;

namespace wavelet {

  ac_ibitstream::ac_ibitstream(istream& i, size_t bb, size_t bs) 
    : in(i), bufsize(bs), 
      bit_model(new Adaptive_Bit_Model()), decoder(new Arithmetic_Codec(bs)), 
      byte_budget(bb), cur_bits(0), 
      bits(0), total_bits(0)
  {
    decoder->start_decoder();
  }
  

  ac_ibitstream::~ac_ibitstream() { 
    
  }


  void ac_ibitstream::read_in() {
    decoder->stop_decoder();

    size_t bytes = vl_read(in);
    size_t enc_bytes = vl_read(in);
    bits = bytes << 3;
    
    if (bytes > bufsize) {
      cerr << "Error: decoder bufsize too small." << endl;
      exit(1);
    }

    in.read((char*)decoder->buffer(), enc_bytes);

    cur_bits = 0;
    decoder->start_decoder();
  }

  
  unsigned ac_ibitstream::get_bit() {
    if (byte_budget && (total_bits >= (byte_budget << 3))) {
      throw byte_budget_exception();
    }
    
    if (cur_bits == bits) {
      read_in();
    }

    cur_bits++;
    total_bits++;
    return decoder->decode(*bit_model);
  }
  
  
  bool ac_ibitstream::good() {
    return (cur_bits < bits || (in.peek() != EOF && in.good()));
  }

  size_t ac_ibitstream::get_in_bytes() {
    return bits_to_bytes(total_bits);
  }


  void ac_ibitstream::next_byte() {
    while (total_bits & 0x7l) get_bit();
  }
  
} // namespace
