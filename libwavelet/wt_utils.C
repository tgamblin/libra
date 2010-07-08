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
#include "wt_utils.h"
#include <iostream>
#include <typeinfo>
#include <cassert>
#include <cstring>
#include <unistd.h>
using namespace std;

#ifdef HAVE_CONFIG_H
#include "config.h"
#ifdef HAVE_MPI
#include "par_ezw_encoder.h"
#endif
#endif // HAVE_CONFIG_H

namespace wavelet {

  std::ostream& operator<<(ostream& out, const relatives& rels) {
    return out << "relatives{ "
               << "parent=" << rels.parent
               << ", node=" << rels.node
               << ", left=" << rels.left
               << ", right=" << rels.right
               << "}" << endl;
  }


  void print_byte(ostream& out, unsigned char byte, int bits) {
    for (int i=7; i >= 0 && (i >= 8-bits); i--) {
      int bit = (byte & (1 << i)) >> i;
      out << bit;
    }
  }
  
  
  void print_bytes(ostream& out, long count, const unsigned char *bytes) {
    for (int i=0; i < count; i++) {
      print_byte(out, bytes[i]);
      out << " ";
    }
    out << endl;
  }
  
  
  void print_bits(ostream& out, long count, const unsigned char *bytes) {
    int bits = count;
    int i = 0;
    while (bits >= 8) {
      print_byte(out, bytes[i++]);
      bits -= 8;
    }
    
    if (bits) {
      print_byte(out, bytes[i], bits);
    }
  }


  /// Does work for get_tree.
  relatives binary_relative_search(int low, int high, int target, int parent) {
    int pos = low + (high - low) / 2;
    
    assert (low <= high);
    if (target == pos) {
      int left = (pos-1 < low) ? -1 : low + ((pos-1) - low) / 2;
      int right = (pos+1 > high) ? -1 : pos+1 + (high - (pos+1)) / 2;
      
      return relatives(parent, pos, left, right);

    } else if (target < pos) {
      return binary_relative_search(low, pos-1, target, pos);
    } else {  // target > pos
      return binary_relative_search(pos+1, high, target, pos);
    }
  }
  
    
  /// Gets relatives for prticular rank in a tree with size elements
  relatives get_bs_relatives(int rank, int size) {
    assert(rank >= 0 && rank < size);
    return binary_relative_search(0, size-1, rank, -1);
  }
  
  

  static int get_msb(int rank) {
    int msb = (sizeof(int) * 8) - 1;
    while(!(rank >> msb) && msb > -1)
      msb--;
    
    return msb;
  }
  
  static int get_child(int rank, int child) {
    int msb, mask, i;
    
    rank++;
    msb = get_msb(rank);
    mask = 0;
    
    for(i = 0; i < msb; i++)
      mask |= (1 << i);
    
    return ((1 << (msb + 1)) | (child << msb) | (rank & mask)) - 1;
  }

  static int get_parent(int rank) {
    int msb, mask, i;
    
    rank++;
    msb = get_msb(rank);
    mask = 0;
    
    for(i = 0; i < msb - 1; i++)
      mask |= (1 << i);
    
    return ((1 << (msb - 1)) | (rank & mask)) - 1;
  }


  relatives get_radix_relatives(int rank, int size) {
    int parent = get_parent(rank);
    if (parent < 0 || parent >= size) parent = -1;

    int left = get_child(rank, 0);
    if (left >= size) left = -1;

    int right = get_child(rank, 1);
    if (right >= size) right = -1;

    return relatives(parent, rank, left, right);
  }




  void insert_bits(unsigned char *dest, const unsigned char *src, long src_bits, long dest_offset) {
    int extra_dest_bits = (dest_offset & 0x7l);
    long dest_bytes = bits_to_bytes(dest_offset);
    long src_bytes  = bits_to_bytes(src_bits);
    
    if (!extra_dest_bits) {
      // if there is not a partial byte at the end of the destination, we don't need
      // to shift.  Just use plain old memcopy.
      memcpy(dest + dest_bytes, src, src_bytes);
      
    } else if (src_bits > 0) {
      // If there IS a partial byte at the end of the destination, we need to shift bytes
      // in the src over before copying them into place.
      unsigned char hi_mask = ~(0xFF >> extra_dest_bits);
      int lshift = 8 - extra_dest_bits;
      
      // first byte is old dest byte plus high bits of first src byte.
      dest[dest_bytes-1] = (dest[dest_bytes-1] & hi_mask) | (src[0] >> extra_dest_bits);
      
      //TODO: fix so that last byte is or'd with dest. Currently we overwrite.
      // middle dest bytes consist of low and high portions of middle src bytes
      long i=0;
      while (i < src_bytes-1) {
	dest[dest_bytes+i] = (src[i] << lshift) | (src[i+1] >> extra_dest_bits);
	i++;
      }
      
      // write trailing bits if we haven't output everything from src yet.
      if ((src_bits - lshift) > (i << 3)) {
	dest[dest_bytes+i] = (src[i] << lshift);
      }
    }
  }

  void insert_bits(unsigned char *dest, const unsigned char *src, long src_bits, long dest_offset, long src_offset) {
    long pre_src_bytes = (src_offset >> 3);
    long extra_src_offset = (src_offset & 0x7l);
    
    if (extra_src_offset == 0) {
      insert_bits(dest, src + pre_src_bytes, src_bits, dest_offset);
      
    } else if (src_bits > 0) {
      unsigned char lo_mask = (0xFF >> extra_src_offset);
      unsigned char extra = (src[pre_src_bytes] & lo_mask) << extra_src_offset;
      
      long bits = min((8-extra_src_offset), src_bits);
      insert_bits(dest, &extra, bits, dest_offset);
      
      if (bits != src_bits) {
	insert_bits(dest, src + pre_src_bytes + 1, src_bits - bits, dest_offset + bits);
      }
    }
  }

  // --- iterator for radix trees --- //
  radix_iterator::radix_iterator(size_t s) : size(s) {
    path.push_back(0);
  }
  
  radix_iterator::~radix_iterator() { }
  
  bool radix_iterator::has_next() {
    return !path.empty();
  }
  
  size_t radix_iterator::next() {
    size_t rank = path.back();
    path.pop_back();
    
    int right = get_child((int)rank, 1);
    int left = get_child((int)rank, 0);
    
    if (right < (int)size && right >= 0) path.push_back(right);
    if (left < (int)size && left >= 0) path.push_back(left);
    
    if (rank > 2000) {
      cerr << "error: " << rank << endl;
      exit(1);
    }
    return rank;
  }
  

  int set_ezw_args(ezw_encoder& encoder, int *argc, char ***argv) {
    int c;
    size_t passes;
    quantized_t scale;
    encoding_t enc;
    char *err;
    int retval = 0;

    while ((c = getopt(*argc, *argv, "vqp:s:e:")) != -1) {
      switch (c) {
      case 'p':
        passes = strtoll(optarg, &err, 10);
        if (*err) return 1;
        encoder.set_pass_limit(passes);
        break;
      case 's':
        scale = strtoll(optarg, &err, 10);
        if (*err) return 1;
        encoder.set_scale(scale);
        break;
      case 'e':
        enc = str_to_encoding(optarg);
        if (enc == NONE) return 1;
        encoder.set_encoding_type(enc);
        break;
      case 'q':
#ifdef HAVE_MPI
        try {
          par_ezw_encoder& par_encoder = dynamic_cast<par_ezw_encoder&>(encoder);
          par_encoder.set_use_sequential_order(true);
        } catch (bad_cast) {
          // do nothing -- ignore.
        }
#endif // HAVE_MPI
        break;
      }
    }

    // adjust params
    *argc -= optind;
    *argv += optind;

    return retval;
  }


  void ezw_usage(const char *app_name, const char *more_args) {
    if (!more_args) more_args = "";

    cerr << "Usage: " << app_name << " [-s scale] [-p passes] [-e encoding] [-q] " << more_args << endl;
    cerr << "  Arguments:" << endl;
    cerr << "  -s    Scale double-precision data by a factor before quantized ezw coding." << endl;
    cerr << "  -p    Limit encoding to first <passes> passes of ezw coded data. " << endl;
    cerr << "  -e    Sets encoding for output data (after RLE coding).  Options: [rle|arithmetic|huffman]." << endl;
    cerr << "  -q    Parallel only.  Require that bits be output in sequential order." << endl;
    cerr << "        Severely impacts performance." << endl;
    
    exit(1);
  }
  
} // namespace

