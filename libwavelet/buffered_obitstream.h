#ifndef BUFFERED_OBITSTREAM_H
#define BUFFERED_OBITSTREAM_H

#include <iostream>
#include <vector>
#include <ostream>
#include "obitstream.h"

namespace wavelet {

  class buffered_obitstream : public obitstream {
  protected:
    std::vector<unsigned char> buf;  /// Buffer for as yet unwritten data
    size_t pos;                      /// Current write index in buffer.
    unsigned char mask;              /// Mask of next bit to write.
    size_t bits;                     /// Total bits written out.
    
    std::ostream& out;               /// Underlying destination for all data.
    
    /// Puts all bits on internal buffer onto the output stream.
    /// Doesn't flush the output stream.  WILL write out trailing bits in
    /// last byte of buffer.
    virtual void my_flush();

  public:
    /// Constructor.  Builds a buffered bit stream to write to the passed-in
    /// output stream.
    buffered_obitstream(std::ostream& out, size_t byte_budget = 0, size_t bufsize = DEFAULT_BIT_BUFSIZE);
    
    /// Destructor.  Flushes underlying ostream.
    virtual ~buffered_obitstream();

    /// Puts a zero at the end of output.
    virtual void put_zero();

    /// Puts a one at the end of output.
    virtual void put_one();

    /// Appends nbits bits from buf to the stream.  Optionally, the bits
    /// can be taken starting from an offset (0-7) within buf.
    virtual void write_bits(const unsigned char *buf, size_t nbits, size_t offset = 0);

    /// Forces a write of all data in the bit buffer.  Note that this 
    /// WILL write out any extra bits at the end of the last byte, as the
    /// underlying output stream can only write entire bytes.
    virtual void flush();


    /// Returns number of bits written to this stream.
    virtual size_t get_in_bits();
    
    
    /// Returns number of bytes written to stream (bits rounded up)
    virtual size_t get_in_bytes();


    /// Returns number of bits written to this stream.
    virtual size_t get_out_bits();
    
    
    /// Returns number of bytes written to stream (bits rounded up)
    virtual size_t get_out_bytes();


    /// Skip to next byte in output.
    virtual void next_byte();

  }; // buffered_obitstream

} // namespace

#endif // BUFFERED_OBITSTREAM_H
