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
#ifndef WT_UTILS_H
#define WT_UTILS_H

#include <ostream>
#include <istream>
#include <vector>
#include "ezw_encoder.h"

namespace wavelet {

  /// Structure for representing relatives in a binary tree
  /// By convention, funcitons that return this should use -1
  /// to indicate relatives that aren't present.
  struct relatives {
    int parent;  /// Rank of parent of node in the tree
    int node;    /// Rank of node in the tree
    int left;    /// Rank of node's left child in the tree
    int right;   /// Rank of node's right child in the tree
    
    relatives(int p, int n, int l, int r) 
      : parent(p), node(n), left(l), right(r) { }
    ~relatives() { }
  };
  
  /// Outputs string representation of relatives struct
  std::ostream& operator<<(std::ostream& out, const relatives& rels);

  void print_byte(std::ostream& out, unsigned char byte, int bits = 8);
  void print_bytes(std::ostream& out, long count, const unsigned char *bytes);
  void print_bits(std::ostream& out, long count, const unsigned char *bytes);  


  /// Use this to search binary search trees defined over ranges of ranks.  For
  /// The binary search tree defined on [low, high], this will find the node
  /// numbered <target> and return a relatives structure with the ranks of its
  /// parents and children.  Parents/children that do not exist will be valud -1.
  /// 
  /// Can use this to define an order-preserving reduction tree over a set of 
  /// ranked processes.
  relatives binary_relative_search(int low, int high, int target, int parent = -1);


  /// This is a convenience interface to binary_relative_search.  It will return a 
  /// relatives structure giving the parent and children of the process <rank> in 
  /// a set of processes numbered [0..size).
  /// Use to build ordered reduction trees.
  relatives get_bs_relatives(int rank, int size);
  relatives get_radix_relatives(int rank, int size);


  struct radix_iterator {
    const size_t size;
    std::vector<size_t> path;

    radix_iterator(size_t size);
    ~radix_iterator();

    bool has_next();
    size_t next();
  };
  

  /// Returns reduction root for binary search tree ordered reduce, given the 
  /// size of the communicator on which the reduction was performed.
  inline int bs_root(int size) {
    return (size-1) / 2;
  }

  /// Returns number of bytes needed to hold particular number of bits,
  /// assuming first bit is byte-aligned.
  inline long bits_to_bytes(long bits) {
    return (bits >> 3) + ((bits & 0x7l) ? 1 : 0);
  }

  
  /// Inserts bits from src into dest at an offset.  
  /// Params:
  /// dest:         buffer containing bits to append to.
  /// src:          buffer containing bits to append to dest
  /// copy_bits:    number of bits from src to append.
  /// dest_offset:  offset (in bits) into destination buffer to start writing
  void insert_bits(unsigned char *dest, const unsigned char *src, long copy_bits, 
		   long dest_offset = 0);


  /// Inserts bits from src into dest at an offset.  
  /// Params:
  /// dest:         buffer containing bits to append to.
  /// src:          buffer containing bits to append to dest
  /// copy_bits:    number of bits from src to append.
  /// dest_offset:  offset (in bits) into destination buffer to start writing
  /// src_offset:   offset (in bits) into src buffer to start reading
  void insert_bits(unsigned char *dest, const unsigned char *src, long copy_bits, 
		   long dest_offset, long src_offset);


  /// Gets options for ezw coding from the command line.
  int set_ezw_args(ezw_encoder& encoder, int *argc, char ***argv);
  void ezw_usage(const char *app_name, const char *more_args = NULL);
  
} // namespace


#endif // WT_UTILS_H
