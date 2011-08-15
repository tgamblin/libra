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
#ifndef EFFORT_RECORD_H
#define EFFORT_RECORD_H

#include <vector>

namespace effort {
  
  /// An effort record keeps track of the value of one particular type of effort
  /// over all timesteps.  These are stored in a map and indexed by effort_keys.
  struct effort_record {
    double current;         /// Current value of this type of effort.
    std::vector<double> values;  /// Effort values indexed by progress step

    /// Constructs an emty effort record
    effort_record() : current(0) { }
    
    /// Constructs an effort record with all values zero
    effort_record(int size) : current(0), values(size, 0) { }
    
    /// Destructor
    ~effort_record() { }
    
    /// Pushes current effort on back of values, resets current effort to zero.
    void commit(size_t progress_count);
    
    /// Gets the effort for a particular progress step.
    double& operator[](int p) { return values[p]; }

    /// Gets the size so far.
    size_t size() { return values.size(); }
    
    
    /// Gets the values.begin()
    std::vector<double>::iterator begin() { return values.begin(); }

    /// Adds to current value of this effort.
    void operator+=(double value) { current += value; }
  };
  
  
} // namespace

#endif // EFFORT_RECORD_H
