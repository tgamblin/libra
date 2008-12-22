#ifndef OBITSTREAM_H
#define OBITSTREAM_H

#include <cstdlib>
#define DEFAULT_BIT_BUFSIZE 65536

namespace wavelet {

  /// Interface class for output bit streams.  Contains default implementations of
  /// some convenience methods.
  class obitstream {
  public:
    /// Cnstructor
    obitstream();

    /// Destructor
    virtual ~obitstream();

    /// Puts a zero at the end of output.
    virtual void put_zero() = 0;

    /// Puts a one at the end of output.
    virtual void put_one() = 0;

    /// Gets number of bits written to this obitstream.
    virtual size_t get_in_bits() = 0;
    
    /// Gets number of bytes written to this obitstream.
    virtual size_t get_in_bytes() = 0;
    
    /// Gets number of bits written out by this obitstream.
    virtual size_t get_out_bits() = 0;
    
    /// Gets number of bytes written out by this obitstream.
    virtual size_t get_out_bytes() = 0;
    
    /// Flushes output to underlying output stream.
    virtual void flush() = 0;

    /// Skips to beginning of next byte, filling with zeros.  If we're already about
    /// to write to the beginning of the next byte, just stay there.
    virtual void next_byte() = 0;
    
    /// Writes raw bytes to the bit stream.  Default implementation just calls 
    /// write_bits with offset zero.  Note that this can lose bit alignment
    virtual void write(const unsigned char *buf, size_t bytes);
    
    /// Writes out all bits in a buffer.  Optionally,  an offset into the buffer 
    /// can be specified as a starting point for reading.  Default implementation
    /// calls put_zero and put_one.
    virtual void write_bits(const unsigned char *buf, size_t nbits, size_t offset = 0);

    /// Convenience method -- default impleemntation just calls put_zero or put_one.
    virtual void put_bit(bool bit);

  }; // obitstream

} // namespace

#endif //OBITSTREAM_H
