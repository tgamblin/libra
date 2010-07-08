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
#ifndef PARALLEL_WT_2D_H
#define PARALLEL_WT_2D_H

#include <mpi.h>
#include <vector>
#include "wavelet.h"
#include "wt_direct.h"

/// This is a parallel implementation of the cdf wavelet transform.
/// This is based on the algorithm described in Nielsen, 2000.
/// TODO: arbitrarily-sized matrices.
/// TODO: make this hierarchical so it can do arbitrary transform levels, 
///       instead of just as far as you can go w/the local data size.
namespace wavelet { 

  class wt_parallel : private wt_1d_direct {
  public:
    /// Constructor -- just delegates to wt_direct.
    wt_parallel(filter_bank& f = filter::getCDF97());

    /// Destructor
    virtual ~wt_parallel();

    /// Forward transform for matrix.  
    /// This is a collective operation -- all processes in the communicator
    /// need to call fwt_2d at the same time.
    /// 
    /// PRE:   mat has the same dimensions for all processes, and contains N/P
    ///        rows of a distributed matrix, where N is total rows and P is 
    ///        process count.  mat's size must be a power of two.
    /// 
    ///        Mat's rows are assumed to be distributed sequentially over all 
    ///        ranks in the communicator provided.  If your data is not laid out
    ///        this way, consider using aggregate(), above, with MPI_Comm_split().
    ///
    ///        Level is the maximum level of the tranform to be conducted.
    /// 
    /// mat:   a boost matrix containing the data to be transformed
    /// level: number of level iterations to perform
    ///        must be <= log2(min(mat.size1(), mat.size2())
    ///
    /// Returns the level of the transform performed.  This may be less than
    /// the level provided, depending on the data's layout 
    int fwt_2d(wt_matrix& mat, int level = -1, MPI_Comm comm = MPI_COMM_WORLD);


    
    int iwt_2d(wt_matrix& mat, int level = -1, MPI_Comm comm = MPI_COMM_WORLD);


    /// Use this function to gather distributed data onto fewer processors.
    /// Use with MPI_Comm_split to perform a number of wavelet transforms 
    /// in parallel.
    /// Given a vector of local data
    /// PRE:  local size is a power of two
    ///       system size is power of two
    /// POST: Data in local is aggregated into mat on all processors where 
    ///       (size % m == set)
    static void aggregate(wt_matrix& mat, std::vector<double>& local, 
			  int m, int set, 
			  std::vector<MPI_Request>& reqs, 
			  MPI_Comm comm = MPI_COMM_WORLD);
    
    /// Inverse of aggregate().
    /// 
    /// Given a matrix full of wavelet data:
    /// PRE:  local size is a power of two, local.size() == mat.size2()
    ///       system size is power of two
    /// POST: Rows of matrix are distributed to all processors where
    ///       (size % m == set)
    static void distribute(wt_matrix& mat, std::vector<double>& local, 
			  int m, int set, 
			  std::vector<MPI_Request>& reqs, 
			  MPI_Comm comm = MPI_COMM_WORLD);
    

    /// Gathers all pieces of a distributed matrix together into a local matrix.
    static void gather(wt_matrix& dest, wt_matrix& mat, 
		       MPI_Comm comm, int root = 0);

    /// Scatters per-process chunks of a matrix out to all members of comm.
    static void scatter(wt_matrix& dest, wt_matrix& mat, 
		       MPI_Comm comm, int root = 0);


    /// Puts a matrix back together after a parallel wavelet transform with fwt_2d.
    /// This just rearranges the rows so that they're in the order we're used to.
    /// This algorithm is O(M*N) for an M row by N column matrix.
    /// POST: mat's elements have been rearranged to the standard wavelet transform order.
    static void reassemble(wt_matrix& mat, int P, int level);


  protected:
    /// Wrapper around wt_1d_direct method that knows about matrix.
    void fwt_row(wt_matrix& mat, size_t row, size_t n) {
      fwt_1d(&mat(row, 0), n);
    }

    /// Wrapper around wt_1d_direct method that knows about matrix.
    void iwt_row(wt_matrix& mat, size_t row, size_t n) {
      iwt_1d(&mat(row, 0), n);
    }

    /// Parallel column transform.  Requires that data be preconditioned by 
    void fwt_col(wt_matrix& mat, size_t col, size_t n);

    /// Parallel column transform.  Requires that data be preconditioned by 
    void iwt_col(wt_matrix& mat, size_t col, size_t n);
    
  private:
    /// Copies one column of local and remote data into temporary array: left 
    /// first, then local, then right.  If this process has no left or right neighbor
    /// then the local data is extended symmetrically to the appropriate side(s).
    void build_temp(wt_matrix& left, wt_matrix& local, wt_matrix& right, 
		    size_t rows, size_t col, int rank, int comm_size, bool interleave = false);


    /// This routine handles data exchange between neighbors in the parallel wavelet transform.
    /// Matrices in the parallel transform are distributed by rows, and this fetches all remote
    /// rows necessary for the local column transform.
    ///
    /// For each process, we send D/2 rows to the right and D/2+1 rows to the left.  Likewise,
    /// we receive D/2+1 rows from the right and D/2 rows from the left.  Rows are not transferred
    /// in their entirety; only data from those columns that are necessary for the transform.
    /// 
    /// This procedure issues requests for these sends/recvs, but does not wait for them to
    /// complete.  Instead, this adds requests to the supplied vector, which the caller can wait
    /// on later.
    ///
    /// Parameters:
    /// left:  destination matrix for columns from caller's left.
    /// local: handle to local data, low and high rows of which ard sent left and right.
    /// right: destination matrix for columns from caller's right.
    /// rows:  number of rows in local data still being transformed.
    /// cols:  number of cols in local data still being transformed.
    /// reqs:  All requests issued here are appended to this vector.
    /// comm:  Communicator on which transform is being performed.
    void fwt_exchange(wt_matrix& left, wt_matrix& local, wt_matrix& right, 
		      size_t rows, size_t cols, std::vector<MPI_Request>& reqs, 
		      MPI_Comm comm);


    /// This routine handles data exchange for the inverse wavelet transform.  Parameters
    /// are as for fwt_exchange, but data laout is slightly different.
    void iwt_exchange(wt_matrix& left, wt_matrix& local, wt_matrix& right, 
		      size_t rows, size_t cols, std::vector<MPI_Request>& reqs, 
		      MPI_Comm comm);
  };
  
} // namespace 

#endif //PARALLEL_WT_2D_H
