#ifndef IBITSTREAM_H
#define IBITSTREAM_H

#include <cstdlib>
namespace wavelet {

  /// Interface class for ibput bit streams.  Contains no implementation.
  class ibitstream {
  public:
    /// Constructor
    ibitstream();

    /// Destructor
    virtual ~ibitstream();

    /// Get a single bit off of the input stream, in the lowest bit 
    /// of the result.
    virtual unsigned get_bit() = 0;
    
    /// Whether data from the bitstream is still valid.
    virtual bool good() = 0;

    /// Reads a block of data directly.  Default implemention calls
    /// get_bit() repeatedly.
    virtual void read(unsigned char *buffer, size_t size);

    /// Number of bytes read in.
    virtual size_t get_in_bytes() = 0;

    /// Skips to next aligned byte in stream.
    virtual void next_byte() = 0;

  }; // ibitstream

} // namespace

#endif // IBITSTREAM_H
