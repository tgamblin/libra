#ifndef BUFFERED_IBITSTREAM_H
#define BUFFERED_IBITSTREAM_H

#include <istream>
#include "ibitstream.h"
#include "buffered_obitstream.h"

namespace wavelet {

  // Class for reading individual bits at a time from an input stream.
  // 
  // TODO: buffer input.
  // TOOD: support reading large numbers of bits at a time.
  class buffered_ibitstream : public ibitstream {
  protected:
    std::istream& in;                /// input stream to read bits from.

    std::vector<unsigned char> buf;  /// Buffer or input characters
    size_t pos;                      /// Position in buffer to read next byte
    unsigned char mask;              /// Mask of next bit to read.
    size_t end;                      /// End of buffer in memory (1 past last byte)

    bool eof;       /// EOF flag
    
    /// Buffers data from underlying stream for reading and resets pointer
    virtual void reload();
    
  public:
    /// Constructs an buffered_ibitstream to read from the prvided input stream.
    /// TODO: support byte budget.
    buffered_ibitstream(std::istream& in, size_t byte_budget = 0, size_t bufsize = DEFAULT_BIT_BUFSIZE);

    /// Destructor; does nothing.
    virtual ~buffered_ibitstream();

    /// Gets a single bit of input.  Left in header to enable inlining.
    unsigned get_bit() {
      if (pos == end) {
	reload();
      }

      unsigned bit = (mask & buf[pos]) ? 1 : 0;
      mask >>= 1;

      if (mask == 0) {
	pos++;
	mask = 0x80;
      }

      return bit;
    }

    /// True if there are more bits to be input.
    virtual bool good() {
      return !(pos == end && eof);
    }

    virtual size_t get_in_bytes() {
      return 0;
    }

    virtual void next_byte();

  }; // buffered_ibitstream

} // namespace

#endif // BUFFERED_IBITSTREAM_H

