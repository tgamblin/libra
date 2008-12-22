#ifndef VECTOR_OBITSTREAM_H
#define VECTOR_OBITSTREAM_H

#include <iostream>
#include <vector>
#include <ostream>
#include "obitstream.h"

namespace wavelet {

  class vector_obitstream : public obitstream {
  protected:
    std::vector<unsigned char>& buf;  /// Buffer for as yet unwritten data
    bool my_vector;                  /// Whether or not the vector should be freed on destruct.
    size_t pos;                      /// Current write index in buffer.
    unsigned char mask;              /// Mask of next bit to write.
    size_t bits;                     /// Total bits written out.

  public:
    /// Constructor.  Builds an obitstream to output to an internal vector.
    vector_obitstream(size_t bufsize = DEFAULT_BIT_BUFSIZE);
    

    /// Constructor.  Builds an obitstream to output to the provided vector.
    /// This will start outputting to the beginning of the vector.
    vector_obitstream(std::vector<unsigned char>& buffer);
    

    /// Destructor.  Flushes underlying ostream.
    virtual ~vector_obitstream();


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

    /// returns pointer to internal buffer
    unsigned char *get_buffer();

    /// Returns internal vector
    std::vector<unsigned char>& get_vector();

    /// Resize the internal buffer.
    void resize(size_t size);
    
    /// Swaps internal buffer out with other.  Destroys contents of other and resets stream.
    void swap(std::vector<unsigned char>& other);
    
  }; // vector_obitstream

} // namespace

#endif // VECTOR_OBITSTREAM_H
