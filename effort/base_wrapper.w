/* -*- C++ -*- */

#include "effort_module.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


/* --- Basic wrappers for init, finalize, and pcontrol in the effort library.   */
/* --- These should be included with all function-specific wrapper libs to make */
/* --- sure things get inited and destroyed properly.                           */

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


