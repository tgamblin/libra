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
#ifndef MPI_PROFILE_H
#define MPI_PROFILE_H

// These macros substitute PMPI calls for all the MPI used in the parallel wavelet
// library.  This makes the library suitable for use in tools, as the calls won't
// interfere with other communication profiling code.
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif //HAVE_CONFIG_H

#ifdef USE_PMPI
#define MPI_Bcast         PMPI_Bcast
#define MPI_Comm_rank     PMPI_Comm_rank
#define MPI_Comm_size     PMPI_Comm_size
#define MPI_Gather        PMPI_Gather
#define MPI_Gatherv       PMPI_Gatherv
#define MPI_Allgather     PMPI_Allgather
#define MPI_Allreduce     PMPI_Allreduce
#define MPI_Irecv         PMPI_Irecv
#define MPI_Isend         PMPI_Isend
#define MPI_Ibsend        PMPI_Ibsend
#define MPI_Recv          PMPI_Recv
#define MPI_Reduce        PMPI_Reduce
#define MPI_Send          PMPI_Send
#define MPI_Type_commit   PMPI_Type_commit
#define MPI_Type_free     PMPI_Type_free
#define MPI_Type_vector   PMPI_Type_vector
#define MPI_Waitall       PMPI_Waitall
#endif //USE_PMPI

#endif //MPI_PROFILE_H
