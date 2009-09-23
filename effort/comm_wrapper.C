
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef PIC
/* For shared libraries, declare these weak and figure out which one was linked
   based on which init wrapper was called.  See mpi_init wrappers.  */
#pragma weak pmpi_init
#pragma weak PMPI_INIT
#pragma weak pmpi_init_
#pragma weak pmpi_init__
#endif /* PIC */

    void pmpi_init(MPI_Fint *ierr);
    void PMPI_INIT(MPI_Fint *ierr);
    void pmpi_init_(MPI_Fint *ierr);
    void pmpi_init__(MPI_Fint *ierr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifndef _EXTERN_C_
#ifdef __cplusplus
#define _EXTERN_C_ extern "C"
#else /* __cplusplus */
#define _EXTERN_C_ 
#endif /* __cplusplus */
#endif /* _EXTERN_C_ */
static int in_wrapper = 0;
/* -*- C++ -*- */

#include "effort_module.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


/* Wrapper for splitting routines. */
/* ================== C Wrappers for MPI_Wait ================== */
_EXTERN_C_ int MPI_Barrier(MPI_Comm arg_0) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Barrier(arg_0);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Barrier(arg_0);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_Wait =============== */
static void MPI_Barrier_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *ierr) { 
    int return_val = 0;
#if (defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Barrier((MPI_Comm)(*arg_0));
#else /* MPI-2 safe call */
    return_val = MPI_Barrier(MPI_Comm_f2c(*arg_0));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_BARRIER(MPI_Fint *arg_0, MPI_Fint *ierr) { 
    MPI_Barrier_fortran_wrapper(arg_0, ierr);
}

_EXTERN_C_ void mpi_barrier(MPI_Fint *arg_0, MPI_Fint *ierr) { 
    MPI_Barrier_fortran_wrapper(arg_0, ierr);
}

_EXTERN_C_ void mpi_barrier_(MPI_Fint *arg_0, MPI_Fint *ierr) { 
    MPI_Barrier_fortran_wrapper(arg_0, ierr);
}

_EXTERN_C_ void mpi_barrier__(MPI_Fint *arg_0, MPI_Fint *ierr) { 
    MPI_Barrier_fortran_wrapper(arg_0, ierr);
}

/* ================= End Wrappers for MPI_Wait ================= */


/* ================== C Wrappers for MPI_Wait ================== */
_EXTERN_C_ int MPI_Wait(MPI_Request *arg_0, MPI_Status *arg_1) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Wait(arg_0, arg_1);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Wait(arg_0, arg_1);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_Wait =============== */
static void MPI_Wait_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *ierr) { 
    int return_val = 0;
#if (defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Wait((MPI_Request*)arg_0, (MPI_Status*)arg_1);
#else /* MPI-2 safe call */
    MPI_Request temp_arg_0;
    MPI_Status temp_arg_1;
    temp_arg_0 = MPI_Request_f2c(*arg_0);
    MPI_Status_f2c(arg_1, &temp_arg_1);
    return_val = MPI_Wait(&temp_arg_0, &temp_arg_1);
    *arg_0 = MPI_Request_c2f(temp_arg_0);
    MPI_Status_c2f(&temp_arg_1, arg_1);
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_WAIT(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *ierr) { 
    MPI_Wait_fortran_wrapper(arg_0, arg_1, ierr);
}

_EXTERN_C_ void mpi_wait(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *ierr) { 
    MPI_Wait_fortran_wrapper(arg_0, arg_1, ierr);
}

_EXTERN_C_ void mpi_wait_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *ierr) { 
    MPI_Wait_fortran_wrapper(arg_0, arg_1, ierr);
}

_EXTERN_C_ void mpi_wait__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *ierr) { 
    MPI_Wait_fortran_wrapper(arg_0, arg_1, ierr);
}

/* ================= End Wrappers for MPI_Wait ================= */




