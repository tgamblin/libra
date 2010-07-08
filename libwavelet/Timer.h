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
#ifndef TIMER_H
#define TIMER_H

#include <vector>
#include <map>
#include <string>
#include <iostream>

#include "timing.h"

class Timer {
  typedef std::map<std::string, timing_t> timing_map;

  timing_map timings;              /// Map from user-supplied keys to elements
  std::vector<std::string> order;  /// Keys into element map, in insertion order
  timing_t start;                  /// Time this Timer was last constructedor cleared.
  timing_t last;                   /// Last time restart() or record() was called.
  
  /// Convenience method for getting elts out of const map.
  timing_t get(const std::string& name) const {
    timing_map::const_iterator i = timings.find(name);
    return (i != timings.end()) ? i->second : 0;
  }

  
public:
  Timer();
  Timer(const Timer& other);
  ~Timer();

  /// Empties out all recorded timings so far AND sets last_time to now.
  void clear();

  /// Skips ahead and sets last time to now.
  void fast_forward();

  /// Records time since start or last call to record.
  void record(const std::string& name);

  /// Appends timings from another timer to those for this one.  Also updates
  /// last according to that of other timer.
  Timer& operator+=(const Timer& other);

  /// Returns when the timer was initially constructed
  timing_t start_time() const { return start; }

  /// Prints all timings (nicely formatted, in sec) to a file.
  void write(std::ostream& out = std::cout, bool print_total = false) const;

  /// Writes AND clears.
  void dump(std::ostream& out = std::cout, bool print_total = false) {
    write(out, print_total);
    clear();
  }
  
  /// Returns the i-th timing recorded (starting with 0)
  timing_t operator[](const std::string& name) const { 
    return get(name); 
  }

  Timer& operator=(const Timer& other);
};

/// Syntactic sugar; calls write on the timer and passes the ostream.
inline std::ostream& operator<<(std::ostream& out, const Timer& timer) {
  timer.write(out);
  return out;
}


#endif // TIMER_H
