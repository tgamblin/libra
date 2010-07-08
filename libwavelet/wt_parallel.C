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
#include "wt_parallel.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
using namespace std;

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H

#include "mpi_profile.h"
#include "cdf97.h"
#include "matrix_utils.h"
#include "mpi_utils.h"

namespace wavelet {

  // Just delegates to superclass.
  wt_parallel::wt_parallel(filter_bank& f) : wt_1d_direct(f) { }

  // Does nothing.
  wt_parallel::~wt_parallel() { }


  int wt_parallel::fwt_2d(wt_matrix& local, int level, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    // This pushes the level as low as possible without requiring 
    // more than nearest-neighbor communication
    if (level < 0) {
      size_t rows = local.size1();
      for (level = 0; rows > f.size/2+1; level++) {
        rows >>= 1;
      }
    }

    // ensure local size is divisible by 2 level times.
    assert(isDivisibleBy2(local.size1(), level));

    wt_matrix left, right;

    for (int l=0; l < level; l++) {
      size_t rows = local.size1() >> l;
      size_t cols = local.size2() >> l;

      // do local transform within rows using convolution method.
      for (size_t r=0; r < rows; r++) {
        fwt_row(local, r, cols);
      }

      // async requests for sends/recvs of remove columns
      vector<MPI_Request> reqs;
      fwt_exchange(left, local, right, rows, cols, reqs, comm);

      // wait on communication (TODO: overlap comm & local computation) 
      MPI_Status statuses[reqs.size()];
      MPI_Waitall(reqs.size(), &reqs[0], statuses);

      size_t tsize = rows + 2 * (f.size/2) + 1;
      if (temp.size() < tsize) temp.resize(tsize);

      // now do all column computations 
      for (size_t c=0; c < cols; c++) {	
        build_temp(left, local, right, rows, c, rank, size);
        fwt_col(local, c, rows);
      }
    }

    // return level so that caller knows what's needed to get a full transform
    return level;
  }


  int wt_parallel::iwt_2d(wt_matrix& local, int level, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    
    // push level as low as possible without losing nearest-neighbor comm
    if (level < 0) {
      size_t rows = local.size1();
      for (level = 0; rows > f.size/2+1; level++) {
        rows >>= 1;
      }
    }

    // ensure divisible by 2 level times.
    assert(isDivisibleBy2(local.size1(), level));

    wt_matrix left, right;

    size_t rows, cols;
    for (int l=level-1; l >= 0; l--) {
      rows = local.size1() >> l;
      cols = local.size2() >> l;

      // async requests for sends/recvs of remove columns
      vector<MPI_Request> reqs;
      iwt_exchange(left, local, right, rows, cols, reqs, comm);

      // wait on communication (TODO: overlap comm & computation) 
      MPI_Status statuses[reqs.size()];
      MPI_Waitall(reqs.size(), &reqs[0], statuses);

      // now do all column computations 
      for (size_t c=0; c < cols; c++) {	
        build_temp(left, local, right, rows, c, rank, size, true);
        iwt_col(local, c, rows);
      }

      // do local iwt within rows using convolution method.
      for (size_t r=0; r < rows; r++) {
        iwt_row(local, r, cols);
      }
    }

    // return level so that caller knows what's needed to get a full transform
    return level;
  }


  void wt_parallel::aggregate(wt_matrix& mat, vector<double>& local, int m, int set,
                              vector<MPI_Request>& reqs, MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);

    int base = (rank / m) * m;  // base rank of the set we're in

    if (rank % m == set) {
      mat.resize(m, local.size());            // allocate space for received data
      
      for (int i=0; i < m; i++) {
        if (i == set) continue;
        reqs.push_back(MPI_REQUEST_NULL);
        MPI_Irecv(&mat(i,0), local.size(), MPI_DOUBLE, base+i, 0, comm, &reqs.back());
      }

      for (size_t i=0; i < local.size(); i++) {  // copy local data into matrix, too
        mat(set, i) = local[i];
      }

    } else {
      // send this process's data to the aggregating process
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Isend(&local[0], local.size(), MPI_DOUBLE, base+set, 0, comm, &reqs.back());
    }
  }


  void wt_parallel::distribute(wt_matrix& mat, vector<double>& local, int m, int set,
                               vector<MPI_Request>& reqs, MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);

    int base = (rank / m) * m;  // base rank of the set we're in

    if (rank % m == set) {
      for (int i=0; i < m; i++) {
        if (i == set) continue;
        reqs.push_back(MPI_REQUEST_NULL);
        MPI_Isend(&mat(i,0), local.size(), MPI_DOUBLE, base+i, 0, comm, &reqs.back());
      }

      for (size_t i=0; i < local.size(); i++) {  // copy local data into matrix, too
        local[i] = mat(set, i);
      }

    } else {
      // send this process's data to the aggregating process
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&local[0], local.size(), MPI_DOUBLE, base+set, 0, comm, &reqs.back());
    }
  }


  void wt_parallel::gather(wt_matrix& mat, wt_matrix& remote, MPI_Comm comm, int root) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    void *recvbuf = NULL;
    if (rank == root) {
      // root needs space enough for all data
      mat.resize(size * remote.size1(), remote.size2());
      recvbuf = &mat(0,0);
    } 

    // gather remote matrices to root
    MPI_Gather(&remote(0,0), remote.size1() * remote.size2(), MPI_DOUBLE,
               recvbuf,      remote.size1() * remote.size2(), MPI_DOUBLE, 
               root, comm);
  }


  void wt_parallel::scatter(wt_matrix& mat, wt_matrix& remote, MPI_Comm comm, int root) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    void *recvbuf = NULL;
    if (rank == root) {
      // root needs space enough for all data
      mat.resize(size * remote.size1(), remote.size2());
      recvbuf = &mat(0,0);
    } 

    // gather remote matrices to root
    MPI_Gather(&remote(0,0), remote.size1() * remote.size2(), MPI_DOUBLE,
               recvbuf,      remote.size1() * remote.size2(), MPI_DOUBLE, 
               root, comm);
  }



  void wt_parallel::reassemble(wt_matrix& mat, int P, int level) {
    size_t rows = mat.size1();
    size_t cols = mat.size2();

    wt_matrix temp = mat;
    size_t S = rows / P;   // rows per process

    // range of columns to process per outer-loop iteration.  
    // (cend-ctart) is a factor of 2 greater each iteration, and
    // the reassembly done for each incremental range of columns is
    // of progressively lower order.
    size_t cstart = 0;
    //size_t cend = cols >> (level-1);

    for (int i=0; i < level; i++) {
      size_t cend = cols >> (level-i-1); // upper bound on columns to swap on this iteration
      size_t L = S >> (level-i);         // number of elements to copy per level
      size_t row = 0;                    // destination row counter for both loops below
      
      // grab sum coefficients from procs
      // these are just at the lowest level.
      for (int p = 0; p < P; p++) {
        for (size_t l=0; l < L; l++) {
          for (size_t c=cstart; c < cend; c++) {
            mat(row, c) = temp(p*S+l, c);
          }
          row++;
        }
      }
      
      // cumulative offsets into each proc's data copied so far
      size_t roff = L;
      for (; L < S; L <<= 1) {
        // grab difference coefficients in proper order
        for (int p = 0; p < P; p++) {
          for (size_t l=0; l < L; l++) {
            for (size_t c=cstart; c < cend; c++) {
              mat(row, c) = temp(p*S+roff+l, c);
            }
            row++;
          }
        }
        roff += L;
      }

      cstart = cend;
    }
  }


  // PRE: temp has been filled in by fwt_2d()
  void wt_parallel::fwt_col(wt_matrix& mat, size_t col, size_t n) {
    assert(!(n&1)); // ensure even number. TODO: necessary?

    size_t len = n >> 1;
    for (size_t i=0; i < len; i++) {
      mat(i, col) = mat(len+i, col) = 0;

      for (size_t d=0; d < f.size; d++) {
        mat(i, col) += f.lpf[d] * temp[2*i+d];
        mat(len+i, col) += f.hpf[d] * temp[2*i+d+1];
      }
    }
  }
  

  // PRE: temp has been filled in by iwt_2d()
  void wt_parallel::iwt_col(wt_matrix& mat, size_t col, size_t n) {
    assert(!(n & 1));

    for (size_t i=0; i < n; i++) {
      mat(i,col) = 0.0;
      for (size_t d=0; d < f.size; d++) {
        // this check upsamples the two bands in the input data
        if ((i+d) & 1) mat(i,col) += f.ihpf[d] * temp[i+d];
        else           mat(i,col) += f.ilpf[d] * temp[i+d];
      }
    }
  }


  void wt_parallel::fwt_exchange(wt_matrix& left, wt_matrix& local, wt_matrix& right, 
                                 size_t rows, size_t cols, vector<MPI_Request>& reqs, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    // create strided datatypes for the rows we'll send.  We only need to 
    // send <cols> columns from each row.
    MPI_Datatype left_type, right_type;
    MPI_Type_vector(f.size/2, cols, local.size2(), MPI_DOUBLE, &left_type);
    MPI_Type_commit(&left_type);

    MPI_Type_vector(f.size/2+1, cols, local.size2(), MPI_DOUBLE, &right_type);
    MPI_Type_commit(&right_type);

    // Now do the sends and receives to both neighbors.
    if (rank-1 >= 0) {                  // exchange border rows w/left neighbor.
      left.resize(f.size/2, local.size2());  // keep cols at full size, to avoid reallocating

      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Isend(&local(0,0), 1, right_type, rank-1, 0, comm, &reqs.back());

      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&left(0,0), 1, left_type, rank-1, 0, comm, &reqs.back());
    }

    if (rank+1 < size) {                   // exchange border rows w/right neighbor.
      right.resize(f.size/2+1, local.size2());  // keep cols at full size, to prevent realloc

      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Isend(&local(rows-f.size/2,0), 1, left_type, rank+1, 0, comm, &reqs.back());
      
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&right(0,0), 1, right_type, rank+1, 0, comm, &reqs.back());
    }

    MPI_Type_free(&left_type);
    MPI_Type_free(&right_type);
  }


  void wt_parallel::iwt_exchange(wt_matrix& left, wt_matrix& local, wt_matrix& right, 
                                 size_t rows, size_t cols, std::vector<MPI_Request>& reqs, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    // create strided datatypes for the rows we'll send in each of the subbands
    // We send with single-column stride and receive 2-column stride, so that the
    // colums from subbands are interleaved on the destination process
    MPI_Datatype long_send_type, short_send_type, long_recv_type, short_recv_type;
    MPI_Type_vector(f.size/4,   cols, local.size2(),   MPI_DOUBLE, &short_send_type);
    MPI_Type_vector(f.size/4+1, cols, local.size2(),   MPI_DOUBLE, &long_send_type);
    MPI_Type_vector(f.size/4,   cols, local.size2()*2, MPI_DOUBLE, &short_recv_type);
    MPI_Type_vector(f.size/4+1, cols, local.size2()*2, MPI_DOUBLE, &long_recv_type);

    MPI_Type_commit(&long_send_type);
    MPI_Type_commit(&short_send_type);
    MPI_Type_commit(&long_recv_type);
    MPI_Type_commit(&short_recv_type);

    // Now do the sends and receives to both neighbors.
    if (rank-1 >= 0) {                       // exchange border rows w/left neighbor.
      left.resize(f.size/2, local.size2());  // keep cols at full size, to avoid reallocating

      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Isend(&local(0,0), 1, long_send_type, rank-1, 0, comm, &reqs.back());
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Isend(&local(rows/2,0), 1, short_send_type, rank-1, 0, comm, &reqs.back());

      // receive rows from left process.  receive automatically interleaves.
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&left(1,0), 1, short_recv_type, rank-1, 0, comm, &reqs.back());
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&left(0,0), 1, short_recv_type, rank-1, 0, comm, &reqs.back());
    }

    if (rank+1 < size) {                        // exchange border rows w/right neighbor.
      right.resize(f.size/2+1, local.size2());  // keep cols at full size, to prevent realloc

      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Isend(&local(rows-f.size/4,0), 1, short_send_type, rank+1, 0, comm, &reqs.back());
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Isend(&local(rows/2-f.size/4,0), 1, short_send_type, rank+1, 0, comm, &reqs.back());

      // receive rows from right process.  receive automatically interleaves.
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&right(0,0), 1, long_recv_type, rank+1, 0, comm, &reqs.back());
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&right(1,0), 1, short_recv_type, rank+1, 0, comm, &reqs.back());
    }

    MPI_Type_free(&long_send_type);
    MPI_Type_free(&short_send_type);
    MPI_Type_free(&long_recv_type);
    MPI_Type_free(&short_recv_type);
  }


  void wt_parallel::build_temp(wt_matrix& left, wt_matrix& local, wt_matrix& right, 
                               size_t n, size_t col, int rank, int comm_size, bool interleave) {
    size_t tsize = n + 2 * (f.size/2) + 1;
    if (temp.size() < tsize) temp.resize(tsize);
    
    // copy data from x into middle of temp
    if (interleave) {
      // this interleaves first and second half of x in temp
      for (size_t i=0; i < n/2; i++) {
        temp[f.size/2+(2*i)] = local(i, col);
        temp[f.size/2+(2*i+1)] = local((n/2+i), col);
      }

    } else {
      // this just copies x straight into temp
      for (size_t i=0; i < n; i++) {
        temp[f.size/2+i] = local(i, col);
      }
    }

    // symmetrically extend left and right border around data
    size_t l = f.size/2-1;
    size_t r = n + f.size/2;
    for (size_t i=1; i<=f.size/2; i++) {
      temp[l] = (rank - 1 >= 0) ? left(l, col) : temp[l+2*i];
      temp[r] = (rank + 1 < comm_size) ? right(r-n-f.size/2, col) : temp[l+n-1];
      l--;
      r++;
    }
    // last elt on right
    temp[r] = (rank + 1 < comm_size) ? right(r-n-f.size/2, col) : temp[l+n-1];
  }

  
} // namespace


