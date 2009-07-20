/* -*- C++ -*- */

#include "effort_module.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


/* Wrapper for splitting routines. */
{{fn fn_name MPI_Alltoall MPI_Allgather MPI_Allgatherv MPI_Allreduce 
             MPI_Barrier MPI_Bcast 
             MPI_Gather MPI_Gatherv 
             MPI_Reduce MPI_Reduce_scatter MPI_Scan MPI_Scatter MPI_Scatterv 
             MPI_Waitall MPI_Waitany MPI_Wait 
             }}
#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    {{callfn}}
    effort_exit_comm();
{{endfn}}
