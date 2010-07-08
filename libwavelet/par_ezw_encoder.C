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
#include "par_ezw_encoder.h"

#include <string>
#include <vector>
#include <fstream>
using namespace std;

#include "mpi_profile.h"
#include "mpi_utils.h"
#include "io_utils.h"
#define MPI_QUANTIZED_T (mpi_typeof(quantized_t()))
#include "wt_utils.h"
#include "vector_obitstream.h"

#include "rle.h"
#include "huffman.h"

namespace wavelet {

  par_ezw_encoder::par_ezw_encoder() : use_sequential_order(false) { }


  par_ezw_encoder::~par_ezw_encoder() { }


  void par_ezw_encoder::set_use_sequential_order(bool use) {
    use_sequential_order = use;
  }

  
  bool par_ezw_encoder::get_use_sequential_order() {
    return use_sequential_order;
  }


  int par_ezw_encoder::get_root(MPI_Comm comm) {
    if (use_sequential_order) {
      int size;
      MPI_Comm_size(comm, &size);
      return bs_root(size);
      
    } else {
      return 0;
    }
  }
  

  static void rle_recv(vector<unsigned char>& data, int src, MPI_Comm comm, vector<MPI_Request>& reqs) {
    size_t size;
    MPI_Status status;
    MPI_Recv(&size, 1, MPI_SIZE_T, src, 0, comm, &status);
    data.resize(size);
    
    reqs.push_back(MPI_REQUEST_NULL);
    MPI_Irecv(&data[0], size, MPI_BYTE, src, 0, comm, &reqs.back());
  }
  
  
  static void rle_gather(vector<unsigned char>& dest, vector<unsigned char>& data,
                         int root = 0, MPI_Comm comm = MPI_COMM_WORLD) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    vector<MPI_Request> reqs;
    
    relatives rels = get_radix_relatives(rank, size);
    
    // first receive rle-encoded data from children
    vector<unsigned char> rle_left;
    if (rels.left >= 0) {
      rle_recv(rle_left, rels.left, comm, reqs);
    }
    
    vector<unsigned char> rle_right;
    if (rels.right >= 0) {
      rle_recv(rle_right, rels.right, comm, reqs);
    }
    
    MPI_Waitall(reqs.size(), &reqs[0], MPI_STATUSES_IGNORE);
    reqs.clear();
    
    // next merge the data received from children in 
    // DF traversal order
    unsigned char *mbufs[3];
    size_t msizes[3];
    size_t count = 0;
    size_t msize = 0;
    
    mbufs[count] = &data[0];
    msizes[count++] = data.size();
    msize += data.size();
    
    if (rels.left >= 0) {
      mbufs[count] = &rle_left[0];
      msizes[count++] = rle_left.size();
      msize += rle_left.size();
    }
    
    if (rels.right >= 0) {
      mbufs[count] = &rle_right[0];
      msizes[count++] = rle_right.size();
      msize += rle_right.size();
    }
    
    vector<unsigned char> merge(msize * 30);
    msize = RLE_Merge(mbufs, msizes, count, &merge[0]);
    merge.resize(msize);
    
    if (rels.parent >= 0) {
      // if we're not the root, send the data along
      size_t size = merge.size();
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Isend(&size, 1, MPI_SIZE_T, rels.parent, 0, comm, &reqs.back());      

      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Isend(&merge[0], size, MPI_BYTE, rels.parent, 0, comm, &reqs.back());

      MPI_Waitall(reqs.size(), &reqs[0], MPI_STATUSES_IGNORE);
      
    } else {
      // this is the root, so put the aggregated data into dest
      merge.swap(dest);
    }
  }


  size_t par_ezw_encoder::bit_stitch_encode(const unsigned char *passes, size_t local_bytes, ostream& out, 
                                            ezw_header& header, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    relatives rels = get_bs_relatives(rank, size);    // relatives in the binary reduction tree    
    const size_t num_passes = dom_sizes.size();

    size_t left_bytes = 0;
    size_t right_bytes = 0;
    vector<size_t> left_dom_sizes(num_passes), left_sub_sizes(num_passes);
    vector<size_t> right_dom_sizes(num_passes), right_sub_sizes(num_passes);
    
    // Now reduce encoded data to the root, merging partial bytes on the edges as we go.
    vector<MPI_Request> reqs;
    vector<MPI_Request> size_reqs;

    // get sizes of data from children so that we can appropriately size buffers.    
    if (rels.left >= 0) {
      size_reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&left_bytes, 1, MPI_SIZE_T, rels.left, 0, comm, &size_reqs.back());

      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&left_dom_sizes[0], num_passes, MPI_SIZE_T, rels.left, 0, comm, &reqs.back());
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&left_sub_sizes[0], num_passes, MPI_SIZE_T, rels.left, 0, comm, &reqs.back());
    }

    if (rels.right >= 0) {
      size_reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&right_bytes, 1, MPI_SIZE_T, rels.right, 0, comm, &size_reqs.back());

      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&right_dom_sizes[0], num_passes, MPI_SIZE_T, rels.right, 0, comm, &reqs.back());
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&right_sub_sizes[0], num_passes, MPI_SIZE_T, rels.right, 0, comm, &reqs.back());
    }

    if (size_reqs.size()) {
      MPI_Waitall(size_reqs.size(), &size_reqs[0], MPI_STATUSES_IGNORE);
      size_reqs.clear();
    }

    vector<unsigned char> left_buf(left_bytes);
    vector<unsigned char> right_buf(right_bytes);    

    // receive buffers from parents in the tree
    if (rels.left >= 0) {
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&left_buf[0], left_bytes, MPI_BYTE, rels.left, 0, comm, &reqs.back());
    }

    if (rels.right >= 0) {
      reqs.push_back(MPI_REQUEST_NULL);
      MPI_Irecv(&right_buf[0], right_bytes, MPI_BYTE, rels.right, 0, comm, &reqs.back());
    }
    
    vector_obitstream merged_bits;

    if (reqs.size()) {
      MPI_Waitall(reqs.size(), &reqs[0], MPI_STATUSES_IGNORE);
    }
    reqs.clear();

    vector<size_t> new_dom_sizes(num_passes);
    vector<size_t> new_sub_sizes(num_passes);

    size_t left_offset = 0;
    size_t local_offset = 0;
    size_t right_offset = 0;

    // need to recreate the threshold so we can skip last subordinate pass
    quantized_t threshold = header.threshold;

    for (size_t i=0; i < num_passes; i++) {
      // Merge the dominant passes first.
      merged_bits.write_bits(&left_buf[0], left_dom_sizes[i], left_offset);
      left_offset += left_dom_sizes[i]; 

      merged_bits.write_bits(passes, dom_sizes[i], local_offset);
      local_offset += dom_sizes[i]; 

      merged_bits.write_bits(&right_buf[0], right_dom_sizes[i], right_offset);
      right_offset += right_dom_sizes[i]; 
      
      new_dom_sizes[i] = (left_dom_sizes[i] + dom_sizes[i] + right_dom_sizes[i]);


      threshold >>= 1;

      if (threshold) {
        // stitch subordinate passes together in proper order (they are accumulated per pass)
        for (size_t p=0; p <= i; p++) {
          // write data from each neighbor.
          size_t left_chunk_size = left_sub_sizes[p] - (p ? left_sub_sizes[p-1] : 0);
          merged_bits.write_bits(&left_buf[0], left_chunk_size, left_offset);
          left_offset += left_chunk_size;
	  
          size_t local_chunk_size = sub_sizes[p] - (p ? sub_sizes[p-1] : 0);
          merged_bits.write_bits(passes, local_chunk_size, local_offset);
          local_offset += local_chunk_size;
	  
          size_t right_chunk_size = right_sub_sizes[p] - (p ? right_sub_sizes[p-1] : 0);
          merged_bits.write_bits(&right_buf[0], right_chunk_size, right_offset);
          right_offset += right_chunk_size;
        }
      }

      new_sub_sizes[i] = (left_sub_sizes[i] + sub_sizes[i] + right_sub_sizes[i]);
    }

    // send bit-concatenated buffer to parent
    size_t merged_size = merged_bits.get_out_bytes();
    if (rels.parent >= 0) {
      int count = 0;
      MPI_Request sends[4];
      
      MPI_Isend(&merged_size, 1, MPI_SIZE_T, rels.parent, 0, comm, &sends[count++]);
      MPI_Isend(&new_dom_sizes[0], num_passes, MPI_SIZE_T, rels.parent, 0, comm, &sends[count++]);
      MPI_Isend(&new_sub_sizes[0], num_passes, MPI_SIZE_T, rels.parent, 0, comm, &sends[count++]);
      MPI_Isend(merged_bits.get_buffer(), merged_size, MPI_BYTE, rels.parent, 0, comm, &sends[count++]);
      MPI_Waitall(count, sends, MPI_STATUSES_IGNORE);
      return 0;

    } else {
      header.ezw_size = merged_size;
      const size_t rle_bound = (size_t)ceil(merged_size * 257.0/256 + 1);
      vector<unsigned char> rle_buffer(rle_bound);
      header.rle_size = RLE_Compress((unsigned char*)merged_bits.get_buffer(), &rle_buffer[0], merged_size);

      return finish_encode(rle_buffer, out, header, true);
    }
  }


  size_t par_ezw_encoder::block_encode(const unsigned char *passes, size_t local_bytes, ostream& out, 
                                       ezw_header& header, MPI_Comm comm) {
    int rank;
    MPI_Comm_rank(comm, &rank);

    const int root = get_root(comm);
    size_t all_bytes;
    MPI_Reduce(&local_bytes, &all_bytes, 1, MPI_SIZE_T, MPI_SUM, root, comm);
    
    // locally run-length encode before parallel merge
    const size_t rle_bound = (size_t)ceil(local_bytes * 257.0/256 + 1);
    vector<unsigned char> rle_buffer(rle_bound);

    const size_t rle_size = RLE_Compress((unsigned char*)passes, &rle_buffer[0], local_bytes);
    rle_buffer.resize(rle_size); // tighten buffer around encoded data.

    timer.record("LocalRLE");
    
    // gather compressed RLE representation w/o decompressing.  Ends of rle buffers
    // are stitched together as the merge goes on.  Note that we'll need to reorder
    // after the merge depending on the layout of the tree.
    vector<unsigned char> gathered;
    rle_gather(gathered, rle_buffer, 0, comm);
    const int all_rle_size = gathered.size();
    
    timer.record("RLEGather");

    if (rank == root) {
      header.ezw_size = all_bytes;
      header.rle_size = all_rle_size;
      return finish_encode(gathered, out, header, true);
    }

    return gathered.size();
  }


  size_t par_ezw_encoder::encode(wt_matrix& mat, ostream& out, int level, MPI_Comm comm) {
    timer.clear();

    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);

    level = get_level(level, mat.size1(), mat.size2());

    // quantize entire matrix
    quantize(mat, scale);

    // get the mean of the quantized matrix to subtract out
    quantized_t total = sum(quantized);

    quantized_t all_total;
    MPI_Allreduce(&total, &all_total, 1, MPI_QUANTIZED_T, MPI_SUM, comm);

    quantized_t elts = (quantized.size1() * quantized.size2() * size);
    quantized_t all_mean = (quantized_t)round(all_total / (double)elts);
    subtract_scalar(all_mean);

    // First set up the header data
    // max and mean need to be computed across entire system.  Allreduces do this here.
    quantized_t abs_max = abs_max_val(quantized);
    quantized_t all_abs_max;
    MPI_Allreduce(&abs_max, &all_abs_max, 1, MPI_QUANTIZED_T, MPI_MAX, comm);

    timer.record("EZWStats");

    // Compute threshold and level in standard way.
    threshold = lePowerOf2((uint64_t)all_abs_max);

    vector_obitstream local_bits(mat.size1() * mat.size2() * sizeof(double));

    // construct header
    ezw_header header(mat.size1() * size, mat.size2(), level, all_mean, scale, threshold, enc_type);

    if (use_sequential_order) {
      // first encode data into a local buffer, but output byte-aligned passes
      do_encode(local_bits, header, false);
      return bit_stitch_encode(local_bits.get_buffer(), local_bits.get_out_bytes(), out, header, comm);

    } else {
      // If we're using plain old radix order, call block_encode and do a distributed RLE reduction.
      header.blocks = size;
      header.passes = pass_limit;

      do_encode(local_bits, header, false);
      timer.record("EZWEncode");

      size_t result = block_encode(local_bits.get_buffer(), local_bits.get_out_bytes(), out, header, comm);
      timer.record("Entropy");
      return result;
    }
  }
} // namespace
