#ifndef VECTOR_IBITSTREAM_H
#define VECTOR_IBITSTREAM_H

#include <istream>
#include "ibitstream.h"
#include "obitstream.h"

namespace wavelet {

  // Class for reading individual bits at a time from an input stream.
  // 
  // TODO: buffer input.
  // TOOD: support reading large numbers of bits at a time.
  class vector_ibitstream : public ibitstream {
  protected:
    const unsigned char *buf;        /// Buffer or input characters
    const size_t end;                /// End of buffer in memory (1 past last byte)
    size_t pos;                      /// Position in buffer to read next byte
    unsigned char mask;              /// Mask of next bit to read.
    
    size_t total_bits;               /// Total bits read in.

  public:
    /// Constructs an vector_ibitstream to read from the prvided input stream.
    /// Params:
    ///    buf   buffer to read from
    ///    size  size in bytes of buffer
    vector_ibitstream(const unsigned char *buf, size_t size);

    /// Destructor; does nothing.
    virtual ~vector_ibitstream();

    /// Gets a single bit of input.  Left in header to enable inlining.
    unsigned get_bit() {
      unsigned bit = (mask & buf[pos]) ? 1 : 0;
      mask >>= 1;

      if (mask == 0) {
	pos++;
	mask = 0x80;
      }

      total_bits++;
      return bit;
    }

    /// True if there are more bits to be input.
    virtual bool good() {
      return (pos < end);
    }


    virtual size_t get_in_bytes();


    virtual void next_byte();

  }; // vector_ibitstream

} // namespace

#endif // VECTOR_IBITSTREAM_H

