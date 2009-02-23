
/* -*- C++ -*- */
#include "effort_module.h"
#include "effort_lock.h"
using namespace effort;

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C" {
  /* Wrapper for splitting routines. */
  {{fn fn_name MPI_Alltoall MPI_Allgather MPI_Allgatherv MPI_Allreduce MPI_Bcast MPI_Gather MPI_Gatherv 
               MPI_Reduce MPI_Reduce_scatter MPI_Scan MPI_Scatterv MPI_Waitall MPI_Waitany}}
    // guard against reentry and don't track effort when we're already inside the library.
  //if (effort_lock::in_effort()) return P{{fn_name}}{{argList}};
  //effort_lock lock;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif

    effort_enter_comm();
    {{callfn}}
    effort_exit_comm();
  {{endfn}}


  /* Wrappers for init, finalize, and pcontrol are special.  They need to call the effort library. */
  {{fn fn_name MPI_Pcontrol}}
  ///if (effort_lock::in_effort()) return P{{fn_name}}{{argList}};
  // effort_lock lock;
    effort_pcontrol{{argList}};
  {{endfn}}


  {{fn fn_name MPI_Init}}
  //    effort_lock lock;
    effort_preinit();
    {{callfn}}
    effort_postinit();
  {{endfn}}


  {{fn fn_name MPI_Finalize}}
  //    effort_lock lock;
    effort_finalize();
    {{callfn}}
  {{endfn}}
} // extern "C"

