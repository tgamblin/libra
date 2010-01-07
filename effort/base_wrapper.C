
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _EXTERN_C_
#ifdef __cplusplus
#define _EXTERN_C_ extern "C"
#else /* __cplusplus */
#define _EXTERN_C_ 
#endif /* __cplusplus */
#endif /* _EXTERN_C_ */

#ifdef MPICH_HAS_C2F
_EXTERN_C_ void *MPIR_ToPointer(int);
#endif // MPICH_HAS_C2F

#ifdef PIC
/* For shared libraries, declare these weak and figure out which one was linked
   based on which init wrapper was called.  See mpi_init wrappers.  */
#pragma weak pmpi_init
#pragma weak PMPI_INIT
#pragma weak pmpi_init_
#pragma weak pmpi_init__
#endif /* PIC */

_EXTERN_C_ void pmpi_init(MPI_Fint *ierr);
_EXTERN_C_ void PMPI_INIT(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init_(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init__(MPI_Fint *ierr);

static int in_wrapper = 0;
/* -*- C++ -*- */

#include "effort_module.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


/* --- Basic wrappers for init, finalize, and pcontrol in the effort library.   */
/* --- These should be included with all function-specific wrapper libs to make */
/* --- sure things get inited and destroyed properly.                           */

/* pcontrol records effort and marks timesteps. */
/* ================== C Wrappers for MPI_Wait ================== */
_EXTERN_C_ int MPI_Pcontrol(const int arg_0, ...) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Pcontrol(arg_0);
    in_wrapper = 1;

    effort_pcontrol(arg_0);
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_Wait =============== */
static void MPI_Pcontrol_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *ierr) { 
    int return_val = 0;
    return_val = MPI_Pcontrol(*arg_0);
    *ierr = return_val;
}

_EXTERN_C_ void MPI_PCONTROL(MPI_Fint *arg_0, MPI_Fint *ierr) { 
    MPI_Pcontrol_fortran_wrapper(arg_0, ierr);
}

_EXTERN_C_ void mpi_pcontrol(MPI_Fint *arg_0, MPI_Fint *ierr) { 
    MPI_Pcontrol_fortran_wrapper(arg_0, ierr);
}

_EXTERN_C_ void mpi_pcontrol_(MPI_Fint *arg_0, MPI_Fint *ierr) { 
    MPI_Pcontrol_fortran_wrapper(arg_0, ierr);
}

_EXTERN_C_ void mpi_pcontrol__(MPI_Fint *arg_0, MPI_Fint *ierr) { 
    MPI_Pcontrol_fortran_wrapper(arg_0, ierr);
}

/* ================= End Wrappers for MPI_Wait ================= */





/* Init effort librarypcontrol records effort */
static int fortran_init = 0;
/* ================== C Wrappers for MPI_Wait ================== */
_EXTERN_C_ int MPI_Init(int *arg_0, char ***arg_1) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Init(arg_0, arg_1);
    in_wrapper = 1;

    effort_preinit();
        if (fortran_init) {
#ifdef PIC
        if (!PMPI_INIT && !pmpi_init && !pmpi_init_ && !pmpi_init__) {
            fprintf(stderr, "ERROR: Couldn't find fortran pmpi_init function.  Link against static library instead.\n");
            exit(1);
        }        switch (fortran_init) {
        case 1: PMPI_INIT(&return_val); break;
        case 2: pmpi_init(&return_val); break;
        case 3: pmpi_init_(&return_val); break;
        case 4: pmpi_init__(&return_val); break;
        default:
            fprintf(stderr, "NO SUITABLE FORTRAN MPI_INIT BINDING\n");
            break;
        }
#else /* !PIC */
        pmpi_init(&return_val);
#endif /* !PIC */
    } else {
        return_val = PMPI_Init(arg_0, arg_1);
    }

    effort_postinit();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_Wait =============== */
static void MPI_Init_fortran_wrapper(MPI_Fint *ierr) { 
    int argc = 0;
    char ** argv = NULL;
    int return_val = 0;
    return_val = MPI_Init(&argc, &argv);
    *ierr = return_val;
}

_EXTERN_C_ void MPI_INIT(MPI_Fint *ierr) { 
    fortran_init = 1;
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init(MPI_Fint *ierr) { 
    fortran_init = 2;
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init_(MPI_Fint *ierr) { 
    fortran_init = 3;
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init__(MPI_Fint *ierr) { 
    fortran_init = 4;
    MPI_Init_fortran_wrapper(ierr);
}

/* ================= End Wrappers for MPI_Wait ================= */





/* Finalize cleans up and compresses data. */
/* ================== C Wrappers for MPI_Wait ================== */
_EXTERN_C_ int MPI_Finalize() { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Finalize();
    in_wrapper = 1;

    effort_finalize();
    return_val = PMPI_Finalize();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_Wait =============== */
static void MPI_Finalize_fortran_wrapper(MPI_Fint *ierr) { 
    int return_val = 0;
    return_val = MPI_Finalize();
    *ierr = return_val;
}

_EXTERN_C_ void MPI_FINALIZE(MPI_Fint *ierr) { 
    MPI_Finalize_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_finalize(MPI_Fint *ierr) { 
    MPI_Finalize_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_finalize_(MPI_Fint *ierr) { 
    MPI_Finalize_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_finalize__(MPI_Fint *ierr) { 
    MPI_Finalize_fortran_wrapper(ierr);
}

/* ================= End Wrappers for MPI_Wait ================= */





