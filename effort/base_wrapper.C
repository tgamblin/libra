
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

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
/* ================== C Wrappers for MPI_Status_set_elements ================== */
_EXTERN_C_ int MPI_Pcontrol(const int arg_0, ...) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Pcontrol(arg_0);
    in_wrapper = 1;

    effort_pcontrol(arg_0);
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_Status_set_elements =============== */
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

/* ================= End Wrappers for MPI_Status_set_elements ================= */





/* Init effort librarypcontrol records effort */
static void (*fortran_init)(MPI_Fint*) = NULL;
/* ================== C Wrappers for MPI_Status_set_elements ================== */
_EXTERN_C_ int MPI_Init(int *arg_0, char ***arg_1) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Init(arg_0, arg_1);
    in_wrapper = 1;

    effort_preinit();
        if (fortran_init) {
#ifdef PIC
        fortran_init(&return_val);
#else /* !PIC */
        pmpi_init_(&return_val);
#endif /* !PIC */
    } else {
        return_val = PMPI_Init(arg_0, arg_1);
    }

    effort_postinit();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_Status_set_elements =============== */
static void MPI_Init_fortran_wrapper(MPI_Fint *ierr) { 
    int argc = 0;
    char ** argv = NULL;
    int return_val = 0;
    return_val = MPI_Init(&argc, &argv);
    *ierr = return_val;
}

_EXTERN_C_ void MPI_INIT(MPI_Fint *ierr) { 
    fortran_init = (void(*)(MPI_Fint*))dlsym(RTLD_NEXT, "PMPI_INIT");
    if (!fortran_init) {
        fprintf(stderr, "ERROR: Couldn't find fortran PMPI_INIT function.  Link against static library instead.\n");
        exit(1);
    }
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init(MPI_Fint *ierr) { 
    fortran_init = (void(*)(MPI_Fint*))dlsym(RTLD_NEXT, "pmpi_init");
    if (!fortran_init) {
        fprintf(stderr, "ERROR: Couldn't find fortran pmpi_init function.  Link against static library instead.\n");
        exit(1);
    }
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init_(MPI_Fint *ierr) { 
    fortran_init = (void(*)(MPI_Fint*))dlsym(RTLD_NEXT, "pmpi_init_");
    if (!fortran_init) {
        fprintf(stderr, "ERROR: Couldn't find fortran pmpi_init_ function.  Link against static library instead.\n");
        exit(1);
    }
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init__(MPI_Fint *ierr) { 
    fortran_init = (void(*)(MPI_Fint*))dlsym(RTLD_NEXT, "pmpi_init__");
    if (!fortran_init) {
        fprintf(stderr, "ERROR: Couldn't find fortran pmpi_init__ function.  Link against static library instead.\n");
        exit(1);
    }
    MPI_Init_fortran_wrapper(ierr);
}

/* ================= End Wrappers for MPI_Status_set_elements ================= */





/* Finalize cleans up and compresses data. */
/* ================== C Wrappers for MPI_Status_set_elements ================== */
_EXTERN_C_ int MPI_Finalize() { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Finalize();
    in_wrapper = 1;

    effort_finalize();
    return_val = PMPI_Finalize();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_Status_set_elements =============== */
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

/* ================= End Wrappers for MPI_Status_set_elements ================= */





