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
#ifndef EFFORT_SIGNATURE_H
#define EFFORT_SIGNATURE_H

#include <cstdlib>
#include <vector>
#include <boost/shared_array.hpp>
#include "libra-config.h"

#ifdef LIBRA_HAVE_MPI
#include <mpi.h>
#endif // LIBRA_HAVE_MPI

#include "matrix_utils.h"

namespace effort {
  
  /// Reference-counted array of effort signature data.
  class effort_signature {
  public:
    /// Construct a new effort signature by transforming the data down to the
    /// specified level and using its low-frequency information.  By default the 
    /// data is transformed to a signature no smaller than 16 elements.
    effort_signature(const double *data, size_t len, int level=-1)  { 
      init(data, len, level); 
    }

    effort_signature(const std::vector<double>& data, int level=-1) { 
      init(&data[0], data.size(), level); 
    }
    
    effort_signature();

    ~effort_signature();

    effort_signature(const effort_signature& other);    
    effort_signature& operator=(const effort_signature& other);    

    double operator[](size_t index) const { return signature[index]; }
    size_t size() const { return sig_size; }
    const double *begin() const { return signature.get(); }
    const double *end() const { return signature.get() + sig_size; }

#ifdef LIBRA_HAVE_MPI
    int packed_size(MPI_Comm comm) const;
    void pack(void *buf, int bufsize, int *position, MPI_Comm comm) const;
    static effort_signature unpack(void *buf, int bufsize, int *position, MPI_Comm comm);
#endif // LIBRA_HAVE_MPI

  private:
    boost::shared_array<double> signature;     /// Transformed Effort data for comparison with
    size_t sig_size;                           /// Length of the shared array.
    
    void init(const double *data, size_t len, int level);
  }; // effort_signature


  // Some functors for comparing effort_signatures
  
  struct sig_euclidean_distance {
    double operator()(const effort_signature& lhs, const effort_signature& rhs) {
      return euclidean_distance(lhs.begin(), lhs.end(), rhs.begin());
    }
  };

  struct sig_manhattan_distance {
    double operator()(const effort_signature& lhs, const effort_signature& rhs) {
      return manhattan_distance(lhs.begin(), lhs.end(), rhs.begin());
    }
  };

} // namespace effort

#endif // EFFORT_SIGNATURE_H
