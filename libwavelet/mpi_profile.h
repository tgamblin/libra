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
