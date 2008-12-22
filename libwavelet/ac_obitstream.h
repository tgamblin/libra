#ifndef AC_OBITSTREAM_H
#define AC_OBITSTREAM_H

#include <ostream>
#include <memory>
#include "buffered_obitstream.h"
#include "byte_budget_exception.h"
class Adaptive_Bit_Model;
class Arithmetic_Codec;

namespace wavelet {
  
  class ac_obitstream : public obitstream {
  protected:
    std::ostream& out;                  /// Output stream to write coded data to.
    const size_t bufsize;                 /// Buffer size for encoder
    const size_t byte_budget;             /// Max number of bytes in output.
    size_t cur_bits;                      /// Bits written out since last flush.

    std::auto_ptr<Adaptive_Bit_Model> bit_model;       /// Bit model for EZW encoding
    std::auto_ptr<Arithmetic_Codec> encoder;           /// Arithmetic coder for the stream
    size_t bits;                          /// Total number of bits written to this stream.
    size_t coded_bytes;                   /// Total number of encoded bytes written out.

  public:
    /// Constructs an arithmetic coder stream to wrap the provided output stream.
    ac_obitstream(std::ostream& out, size_t byte_budget = 0, size_t bufsize = DEFAULT_BIT_BUFSIZE);

    /// Destructor does nothing
    virtual ~ac_obitstream();

        /// Puts a zero at the end of output.
    virtual void put_zero();

    /// Puts a one at the end of output.
    virtual void put_one();

    /// Skip to beginning of next byte in output.
    virtual void next_byte();

    /// Stops encoder and writes encoded bytes to output stream.
    virtual void flush();

    /// Number of bits written to stream
    virtual size_t get_in_bits();

    /// Number of bytes written to stream
    virtual size_t get_in_bytes();

    /// Number of coded bits written out (by flushes)
    virtual size_t get_out_bits();
    
    /// Number of coded bytes written out (by flushes)
    virtual size_t get_out_bytes();

    virtual void write_bits(const unsigned char *src, size_t src_bits, size_t src_offset = 0);
  };
  
}

#endif // AC_OBITSTREAM_H
