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
#ifndef STL_UTILS
#define STL_UTILS

#include <utility>
#include <vector>
#include <iostream>

// ====================================================================================
// stl_utils.h
// This file contains some utility functions for dealing with the STL.
// ====================================================================================


/// Functor to get the first element of a pair.  Use with STL functions like transform().
struct get_first {
  template <typename P>
  typename P::first_type operator()(const P& pair) {
    return pair.first;
  }
};

/// Functor to get the second element of a pair.  Use with STL functions like transform().
struct get_second {
  template <typename P>
  typename P::second_type operator()(const P& pair) {
    return pair.second;
  }
};


template <typename Indexable>
struct indexed_lt_functor {
  const Indexable& container;
  indexed_lt_functor(const Indexable& c) : container(c) { }
  template <typename I>
  bool operator()(const I& lhs, const I& rhs) {
    return container[lhs] < container[rhs];
  }
};

template <typename Indexable>
indexed_lt_functor<Indexable> indexed_lt(const Indexable& container) {
  return indexed_lt_functor<Indexable>(container);
}


template <typename Index>
void invert(std::vector<Index>& vec) {
  std::vector<Index> inverse(vec.size());
  for (size_t i=0; i < vec.size(); i++) {
    inverse[vec[i]] = i;
  }
  inverse.swap(vec);
}


///
/// Generator object for a strided sequence of ints.
///
struct sequence {
  int value, stride;

  sequence(int _start=0, int _stride=1) 
    : value(_start), stride(_stride) { }

  int operator()() {
    int result = value;
    value += stride;
    return result;
  }
};




#endif // STL_UTILS
