#ifndef IAC_STREAM_H
#define IAC_STREAM_H

#include <istream>
#include <memory>
#include "ibitstream.h"
#include "buffered_ibitstream.h"
class Adaptive_Bit_Model;
class Arithmetic_Codec;

namespace wavelet {

  class ac_ibitstream : public ibitstream {
  protected:
    std::istream& in;
    const size_t bufsize;                         /// Size of decoder buffer
    std::auto_ptr<Adaptive_Bit_Model> bit_model;  /// Bit model for EZW encoding
    std::auto_ptr<Arithmetic_Codec> decoder;      /// Arithmetic coder for the stream

    /// Max bytes to read in before throwing exception, or 0 if unlimited.
    const size_t byte_budget;

    size_t cur_bits;      /// current bits read from buffer
    size_t bits;          /// bits in buffer
    size_t total_bits;    /// total bits read in so far

    /// Reads a single encoded block from the input stream.
    void read_in();

  public:
    /// Construct a coded bitstream to read from the input stream.
    ac_ibitstream(std::istream& in, size_t byte_budget = 0, size_t bufsize = DEFAULT_BIT_BUFSIZE);

    /// Destructor
    virtual ~ac_ibitstream();

    /// Returns next bit from stream in the lowest bit of the result.
    virtual unsigned get_bit();
    
    /// True if input is still avaialble.
    virtual bool good();

    /// Gets number of bits read in from underlying stream.
    virtual size_t get_in_bytes();

    /// Skip to next aligned byte.
    virtual void next_byte();
    
  };

} // namespace

#endif // IAC_STREAM_H
