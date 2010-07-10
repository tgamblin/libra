/*************************************************************************
 * Name:        rle.c
 * Author:      Marcus Geelnard
 * Description: RLE coder/decoder implementation.
 * Reentrant:   Yes
 *
 * RLE (Run Length Encoding) is the simplest possible lossless compression
 * method. Nevertheless it serves a purpose, even in state of the art
 * compression (it is used in JPEG compression, for instance). The basic
 * principle is to identify sequences of equal bytes, and replace them with
 * the byte in question and a repetition count (coded in some clever
 * fashion).
 *
 * There are several different ways to do RLE. The particular method
 * implemented here is a very efficient one. Instead of coding runs for
 * both repeating and non-repeating sections, a special marker byte is
 * used to indicate the start of a repeating section. Non-repeating
 * sections can thus have any length without being interrupted by control
 * bytes, except for the rare case when the special marker byte appears in
 * the non-repeating section (which is coded with at most two bytes). For
 * optimal efficiency, the marker byte is chosen as the least frequent
 * (perhaps even non-existent) symbol in the input stream.
 *
 * Repeating runs can be as long as 32768 bytes. Runs shorter than 129
 * bytes require three bytes for coding (marker + count + symbol), whereas
 * runs longer than 128 bytes require four bytes for coding (marker +
 * counthi|0x80 + countlo + symbol). This is normally a win in compression,
 * and it's very seldom a loss of compression ratio compared to using a
 * fixed coding of three bytes (which allows coding a run of 256 bytes in
 * just three bytes).
 *
 * With this scheme, the worst case compression result is
 * (257/256)*insize + 1.
 *
 *-------------------------------------------------------------------------
 * Note: This code is based on the code found in "codrle2.c" and
 * "dcodrle2.c" by David Bourgin, as described in "Introduction to the
 * losslessy compression schemes", 1994. The main differences from Davids
 * implementation are the addition of long (15-bit) run counts, the removal
 * of file I/O (this implementation works solely with preallocated memory
 * buffers), and that the code is now 100% reentrant.
 *-------------------------------------------------------------------------
 * Copyright (c) 2003-2006 Marcus Geelnard
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 * Marcus Geelnard
 * marcus.geelnard at home.se
 *************************************************************************/
#include <string.h>
#include <stdio.h>
#include "rle.h"
/*************************************************************************
 *                           INTERNAL FUNCTIONS                           *
 *************************************************************************/


/*************************************************************************
 * _RLE_WriteRep() - Encode a repetition of 'symbol' repeated 'count'
 * times.
 *************************************************************************/

static size_t _RLE_WriteRep( unsigned char *out, size_t *outpos,
                             unsigned char marker, unsigned char symbol, size_t count )
{
  size_t i, idx;

  idx = *outpos;
  if( count <= 3 )
  {
    if( symbol == marker )
    {
      out[ idx ++ ] = marker;
      out[ idx ++ ] = count-1;
    }
    else
    {
      for( i = 0; i < count; ++ i )
      {
        out[ idx ++ ] = symbol;
      }
    }
  }
  else
  {
    out[ idx ++ ] = marker;
    -- count;
    if( count >= 128 )
    {
      out[ idx ++ ] = (count >> 8) | 0x80;
    }
    out[ idx ++ ] = count & 0xff;
    out[ idx ++ ] = symbol;
  }

  size_t ret = idx - *outpos;
  *outpos = idx;
  return ret;
}


/*************************************************************************
 * _RLE_WriteNonRep() - Encode a non-repeating symbol, 'symbol'. 'marker'
 * is the marker symbol, and special care has to be taken for the case
 * when 'symbol' == 'marker'.
 *************************************************************************/

static void _RLE_WriteNonRep( unsigned char *out, size_t *outpos,
                              unsigned char marker, unsigned char symbol )
{
  size_t idx;

  idx = *outpos;
  if( symbol == marker )
  {
    out[ idx ++ ] = marker;
    out[ idx ++ ] = 0;
  }
  else
  {
    out[ idx ++ ] = symbol;
  }
  *outpos = idx;
}



/*************************************************************************
 *                            PUBLIC FUNCTIONS                            *
 *************************************************************************/


/*************************************************************************
 * RLE_Compress() - Compress a block of data using an RLE coder.
 *  in     - Input (uncompressed) buffer.
 *  out    - Output (compressed) buffer. This buffer must be 0.4% larger
 *           than the input buffer, plus one byte.
 *  insize - Number of input bytes.
 * The function returns the size of the compressed data.
 *************************************************************************/

size_t RLE_Compress( unsigned char *in, unsigned char *out,
                     size_t insize )
{
  unsigned char byte1, byte2, marker;
  size_t  inpos, outpos, count, i, histogram[ 256 ];

  /* Do we have anything to compress? */
  if( insize < 1 ) {
    return 0;
  }

  /* Create histogram */
  for( i = 0; i < 256; ++ i ) {
    histogram[ i ] = 0;
  }
  for( i = 0; i < insize; ++ i ) {
    ++ histogram[ in[ i ] ];
  }

  /* Find the least common byte, and use it as the repetition marker */
  marker = 0;
  for( i = 1; i < 256; ++ i ) {
    if( histogram[ i ] < histogram[ marker ] ) {
      marker = i;
    }
  }

  /* Remember the repetition marker for the decoder */
  out[ 0 ] = marker;
  outpos = 1;

  /* Start of compression */
  byte1 = in[ 0 ];
  inpos = 1;
  count = 1;

  /* Are there at least two bytes? */
  if( insize >= 2 ) {
    byte2 = in[ inpos ++ ];
    count = 2;

    /* Main compression loop */
    do {
      if( byte1 == byte2 ) {
        /* Do we meet only a sequence of identical bytes? */
        while( (inpos < insize) && (byte1 == byte2) && (count < 32768) ) {
          byte2 = in[ inpos ++ ];
          ++ count;
        }
        if( byte1 == byte2 ) {
          _RLE_WriteRep( out, &outpos, marker, byte1, count );
          if( inpos < insize ) {
            byte1 = in[ inpos ++ ];
            count = 1;
          } else {
            count = 0;
          }

        } else {
          _RLE_WriteRep( out, &outpos, marker, byte1, count-1 );
          byte1 = byte2;
          count = 1;
        }

      } else {
        /* No, then don't handle the last byte */
        _RLE_WriteNonRep( out, &outpos, marker, byte1 );
        byte1 = byte2;
        count = 1;
      }

      if( inpos < insize ) {
        byte2 = in[ inpos ++ ];
        count = 2;
      }
    }
    while( (inpos < insize) || (count >= 2) );
  }

  /* One byte left? */
  if( count == 1 ) {
    _RLE_WriteNonRep( out, &outpos, marker, byte1 );
  }

  return outpos;
}


/*************************************************************************
 * RLE_Uncompress() - Uncompress a block of data using an RLE decoder.
 *  in      - Input (compressed) buffer.
 *  out     - Output (uncompressed) buffer. This buffer must be large
 *            enough to hold the uncompressed data.
 *  insize  - Number of input bytes.
 *************************************************************************/
size_t RLE_Uncompress( unsigned char *in, unsigned char *out, size_t insize ) 
{
  unsigned char marker, symbol;
  size_t  i, inpos, outpos, count;

  /* Do we have anything to uncompress? */
  if( insize < 1 ) {
    return 0;
  }

  /* Get marker symbol from input stream */
  inpos = 0;
  marker = in[ inpos ++ ];

  /* Main decompression loop */
  outpos = 0;
  do {
    symbol = in[ inpos ++ ];
    if( symbol == marker ) {
      /* We had a marker byte */
      count = in[ inpos ++ ];

      if( count <= 2 ) {
        /* Counts 0, 1 and 2 are used for marker byte repetition
           only */
        for( i = 0; i <= count; ++ i ) {
          out[ outpos ++ ] = marker;
        }

      } else {
        if( count & 0x80 ) {
          count = ((count & 0x7f) << 8) + in[ inpos ++ ];
        }
        symbol = in[ inpos ++ ];

        for( i = 0; i <= count; ++ i ) {
          out[ outpos ++ ] = symbol;
        }
      }

    } else {
      /* No marker, plain copy */
      out[ outpos ++ ] = symbol;
    }
  }
  while( inpos < insize );

  return outpos;
}



/*************************************************************************
 * Increments values in histogram according to bytes and counts in the
 * Run-length encoded input buffer.
 * PRE: buffer is valid run-length encoded data.
 * POST: histogram's values reflect data in run-length encoded buffer.
 *************************************************************************/
static void Add_to_Histo(size_t *histogram, unsigned char *in, size_t insize) {
  if (insize == 0) return;

  size_t count;

  /* First get marker symbol from input stream */
  size_t inpos = 0;
  unsigned char marker = in[inpos++];

  /* This is like the decompression loop, but it doesn't write full-size data.  
   * It's O(compressed size) */
  do {
    unsigned char symbol = in[inpos++];

    if( symbol == marker ) {
      /* We had a marker byte */
      count = in[inpos ++];
      if (count <= 2) {
        /* Counts 0, 1 and 2 are used for marker byte repetition only */
        histogram[marker] += count;

      } else {
        if( count & 0x80 ) {
          count = ((count & 0x7f) << 8) + in[ inpos ++ ];
        }
        symbol = in[inpos++];
        histogram[symbol] += count;
      }

    } else {
      /* No marker, just increment by one. */
      histogram[symbol] ++;
    }
  }
  while( inpos < insize );
}



/* Parses a single RLE record into a count and a symbol.  
   Returns the length of the parsed region. 
*/
static size_t RLE_Parse(unsigned char *in, size_t insize, unsigned char marker,
                        unsigned char *s, size_t *c) {
  size_t inpos = 0;
  unsigned count;
  unsigned char symbol = in[inpos++];

  if(symbol == marker) {
    count = in[inpos++];
    if (count <= 2) {
      /* Low counts are for marker repetitions. */
      symbol = marker;

    } else {
      /* Check for high counts here. */
      if (count & 0x80) {
        count = ((count & 0x7f) << 8) + in[ inpos ++ ];
      }
      symbol = in[inpos++];
    }

  } else {
    count = 0;

    /* this loop eats low-count runs if there are any. */
    while (inpos < insize && in[inpos] == symbol) {
      inpos++;
      count++;
    }
  }

  *s = symbol;
  *c = count + 1;
  return inpos;
}



/*************************************************************************
 * Rewrites an RLE input stream using a new marker.
 * 
 * PRE:  Input buffer is valid run-length encoded data, not including 
 *       first byte for marker.
 * 
 * POST: input buffer is rewritten onto the output stream using the new
 *       marker provided.  Note that the new marker is not written to the
 *       output stream, just the data.
 * 
 *************************************************************************/
size_t RLE_Rewrite(unsigned char *in, size_t insize, unsigned char *out, 
                   unsigned char marker, unsigned char newmarker, size_t *last) {
  size_t outpos = 0;
  size_t inpos = 0;

  /* Read as though we're decompressing the old stream.  */
  unsigned char symbol;
  size_t count;
  size_t last_record = outpos;

  while (inpos < insize) {
    last_record = outpos;
    size_t size = RLE_Parse(&in[inpos], insize - inpos, marker, &symbol, &count);
    inpos += size;

    if (count == 1) {
      _RLE_WriteNonRep(out, &outpos, newmarker, symbol);
    } else {
      _RLE_WriteRep(out, &outpos, newmarker, symbol, count);
    }
  }
  
  if (last) *last = last_record;
  return outpos;
}


size_t RLE_Merge(unsigned char **bufs, size_t *sizes, size_t num_bufs, unsigned char *out) {
  if (num_bufs == 0) return 0;    // nothing to merge; just return zero.

  size_t histogram[256];

  // create histo from values & counts in all streams
  for(size_t i = 0; i < 256; ++i) histogram[i] = 0;

  // add counts from buffers to new histogram.
  for (size_t i=0; i < num_bufs; i++) Add_to_Histo(histogram, bufs[i], sizes[i]);

  // Find the least common byte, and use it as the repetition marker 
  unsigned char new_marker = 0;
  for (int i = 1; i < 256; ++i) {
    if(histogram[i] < histogram[new_marker]) {
      new_marker = i;
    }
  }
  
  unsigned char sym1, sym2;
  size_t count1 = 0, count2;

  size_t i = 0;
  while (i < num_bufs && sizes[i] < 2) i++;  // skip any empty buffers
  if (i == num_bufs) return 0;
	
  // parse first record out of first non-empty buffer
  size_t inpos = 1;
  inpos += RLE_Parse(&bufs[i][inpos], sizes[i] - inpos, bufs[i][0], &sym1, &count1);

  // parse remaining runs and merge boundaries
  size_t outpos = 0;
  out[outpos++] = new_marker;
	
  while (i < num_bufs) {
    while (inpos < sizes[i]) {
      inpos += RLE_Parse(&bufs[i][inpos], sizes[i] - inpos, bufs[i][0], &sym2, &count2);

      if (sym1 == sym2) {
        count1 += count2;
      } else {
        if (count1 == 1) {
          _RLE_WriteNonRep(out, &outpos, new_marker, sym1);
        } else {
          int remaining = count1;
          while (remaining > 1) {
            _RLE_WriteRep(out, &outpos, new_marker, sym1, (remaining <= 32768 ? remaining : 32768));
            remaining -= 32768;
          }
          if (remaining > 0) _RLE_WriteNonRep(out, &outpos, new_marker, sym1);
        }
        sym1 = sym2;
        count1 = count2;
      }
    }
    i++;
    inpos = 1;
  }

  if (count1 == 1) {
    _RLE_WriteNonRep(out, &outpos, new_marker, sym1);
  } else {
    int remaining = count1;
    while (remaining > 1) {
      _RLE_WriteRep(out, &outpos, new_marker, sym1, (remaining <= 32768 ? remaining : 32768));
      remaining -= 32768;
    }
    if (remaining > 0) _RLE_WriteNonRep(out, &outpos, new_marker, sym1);
  }
	
  return outpos;
}






















