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
#include "effort_signature.h"

#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef HAVE_MPI
#include <mpi.h>
#include "mpi_utils.h"
#endif // HAVE_MPI

#include "wt_lift.h"
#include "io_utils.h"


using namespace wavelet;
using namespace std;

namespace effort {

  void effort_signature::init(const double *data, size_t len, int level) {
    vector<double> tmp(len);
    copy(data, data+len, tmp.begin());

    int max_level = (int)log2pow2(len);
    if (level < 0) {
      level = max_level - 4;
    }

    // transform data
    wt_lift wt;
    wt.fwt_1d(&tmp[0], tmp.size(), level);

    // copy lowest band of transformed coefficients into signature array.
    sig_size = len >> level;
    double *sig = new double[sig_size];
    copy(tmp.begin(), tmp.begin() + sig_size, sig);

    // finally give the shared array ownership.
    signature.reset(sig);
  }


  effort_signature::effort_signature() : sig_size(0) { }

  effort_signature::effort_signature(const effort_signature& other) 
    : signature(other.signature), sig_size(other.sig_size) { }

  effort_signature::~effort_signature() { }

  effort_signature& effort_signature::operator=(const effort_signature& other) {
    signature = other.signature;
    sig_size = other.sig_size;
    return *this;
  }


#ifdef HAVE_MPI
  int effort_signature::packed_size(MPI_Comm comm) const {
    int size = 0;
    size += mpi_packed_size(1, MPI_SIZE_T, comm);
    size += mpi_packed_size(sig_size, MPI_DOUBLE, comm);
    return size;
  }

  void effort_signature::pack(void *buf, int bufsize, int *position, MPI_Comm comm) const {
    PMPI_Pack(const_cast<size_t*>(&sig_size), 1, MPI_SIZE_T, buf, bufsize, position, comm);
    PMPI_Pack(const_cast<double*>(signature.get()), sig_size, MPI_DOUBLE, buf, bufsize, position, comm);
  }

  effort_signature effort_signature::unpack(void *buf, int bufsize, int *position, MPI_Comm comm) {
    effort_signature result;
    PMPI_Unpack(buf, bufsize, position, &result.sig_size, 1, MPI_SIZE_T, comm);

    double *sig = new double[result.sig_size];
    PMPI_Unpack(buf, bufsize, position, sig, result.sig_size, MPI_DOUBLE, comm);
    result.signature.reset(sig);

    return result;
  }
#endif // HAVE_MPI

} // namespace effort
