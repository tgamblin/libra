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
#ifndef COUNTER_ITERATOR_H
#define COUNTER_ITERATOR_H

#include <cstdlib>
#include <iterator>

namespace cluster {
  /// Counting output iterator.  Stores (in your own size_t somewhere) how
  /// many elements were output, but assignment is a no-op.  Usage:
  ///
  /// size_t count;
  /// vector<int> test;
  /// test.push_back(1);
  /// test.push_back(2);
  /// test.push_back(3);
  /// copy(test.begin(), test.end(), counter<int>(count));
  /// 
  /// POST: count is 3, since 3 items were inserted.
  ///
  template <class T>
  struct counter {
    typedef T                        value_type;
    typedef T*                       pointer;
    typedef T&                       reference;
    typedef size_t                   difference_type;
    typedef std::output_iterator_tag iterator_category;
    
    struct target {
      void operator=(T t) { }  // no-op; assignment to target does nothing.
    };
    
    size_t *count;
    counter(size_t& c) : count(&c) { *count = 0; }
    counter(const counter& other) : count(other.count) { }
    counter& operator=(const counter& other) { count = other.count; return *this; }
    target operator*()       { return target(); }
    counter& operator++()    { (*count)++; return *this; }
    counter  operator++(int) { (*count)++; return *this; }
  };

} // namespace cluster
  
#endif // COUNTER_ITERATOR_H
