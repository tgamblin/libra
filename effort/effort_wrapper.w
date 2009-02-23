/* -*- C++ -*- */
#include "effort_module.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C" {
  /* Wrapper for splitting routines. */
  {{fn fn_name MPI_Alltoall MPI_Allgather MPI_Allgatherv MPI_Allreduce MPI_Bcast MPI_Gather MPI_Gatherv 
               MPI_Reduce MPI_Reduce_scatter MPI_Scan MPI_Scatterv MPI_Waitall MPI_Waitany}}
#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    {{callfn}}
    effort_exit_comm();
  {{endfn}}


  /* --- Wrappers for init, finalize, and pcontrol are special. --- */

  /* pcontrol records effort and marks timesteps. */
  {{fn fn_name MPI_Pcontrol}}
    effort_pcontrol{{argList}};
  {{endfn}}


  /* Init effort librarypcontrol records effort */
  {{fn fn_name MPI_Init}}
    effort_preinit();
    {{callfn}}
    effort_postinit();
  {{endfn}}


  /* Finalize cleans up and compresses data. */
  {{fn fn_name MPI_Finalize}}
    effort_finalize();
    {{callfn}}
  {{endfn}}
} // extern "C"

