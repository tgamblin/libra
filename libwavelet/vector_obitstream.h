/////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010, Lawrence Livermore National Security, LLC.  
// Produced at the Lawrence Livermore National Laboratory  
// Written by Todd Gamblin, tgamblin@llnl.gov.
// LLNL-CODE-417602
// All rights reserved.  
// 
// This file is part of Libra. For details, see http://github.com/tgamblin/libra.
// Please also read the LICENSE file for further information.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the disclaimer below.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the disclaimer (as noted below) in the documentation and/or other materials
//    provided with the distribution.
//  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
//    or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
/////////////////////////////////////////////////////////////////////////////////////////////////
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
