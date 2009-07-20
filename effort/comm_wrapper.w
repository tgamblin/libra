/* -*- C++ -*- */

#include "effort_module.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


/* Wrapper for splitting routines. */
{{fn fn_name MPI_Barrier MPI_Wait}}
#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    {{callfn}}
    effort_exit_comm();
{{endfn}}

