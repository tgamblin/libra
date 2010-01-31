
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


/* Wrapper for splitting routines. */
/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Alltoall(void *arg_0, int arg_1, MPI_Datatype arg_2, void *arg_3, int arg_4, MPI_Datatype arg_5, MPI_Comm arg_6) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Alltoall(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Alltoall(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Alltoall_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Alltoall((void*)arg_0, *arg_1, (MPI_Datatype)(*arg_2), (void*)arg_3, *arg_4, (MPI_Datatype)(*arg_5), (MPI_Comm)(*arg_6));
#else /* MPI-2 safe call */
    return_val = MPI_Alltoall((void*)arg_0, *arg_1, MPI_Type_f2c(*arg_2), (void*)arg_3, *arg_4, MPI_Type_f2c(*arg_5), MPI_Comm_f2c(*arg_6));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_ALLTOALL(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    MPI_Alltoall_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, ierr);
}

_EXTERN_C_ void mpi_alltoall(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    MPI_Alltoall_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, ierr);
}

_EXTERN_C_ void mpi_alltoall_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    MPI_Alltoall_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, ierr);
}

_EXTERN_C_ void mpi_alltoall__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    MPI_Alltoall_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Allgather(void *arg_0, int arg_1, MPI_Datatype arg_2, void *arg_3, int arg_4, MPI_Datatype arg_5, MPI_Comm arg_6) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Allgather(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Allgather(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Allgather_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Allgather((void*)arg_0, *arg_1, (MPI_Datatype)(*arg_2), (void*)arg_3, *arg_4, (MPI_Datatype)(*arg_5), (MPI_Comm)(*arg_6));
#else /* MPI-2 safe call */
    return_val = MPI_Allgather((void*)arg_0, *arg_1, MPI_Type_f2c(*arg_2), (void*)arg_3, *arg_4, MPI_Type_f2c(*arg_5), MPI_Comm_f2c(*arg_6));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_ALLGATHER(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    MPI_Allgather_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, ierr);
}

_EXTERN_C_ void mpi_allgather(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    MPI_Allgather_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, ierr);
}

_EXTERN_C_ void mpi_allgather_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    MPI_Allgather_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, ierr);
}

_EXTERN_C_ void mpi_allgather__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    MPI_Allgather_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Allgatherv(void *arg_0, int arg_1, MPI_Datatype arg_2, void *arg_3, int *arg_4, int *arg_5, MPI_Datatype arg_6, MPI_Comm arg_7) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Allgatherv(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Allgatherv(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Allgatherv_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Allgatherv((void*)arg_0, *arg_1, (MPI_Datatype)(*arg_2), (void*)arg_3, (int*)arg_4, (int*)arg_5, (MPI_Datatype)(*arg_6), (MPI_Comm)(*arg_7));
#else /* MPI-2 safe call */
    return_val = MPI_Allgatherv((void*)arg_0, *arg_1, MPI_Type_f2c(*arg_2), (void*)arg_3, (int*)arg_4, (int*)arg_5, MPI_Type_f2c(*arg_6), MPI_Comm_f2c(*arg_7));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_ALLGATHERV(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    MPI_Allgatherv_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, ierr);
}

_EXTERN_C_ void mpi_allgatherv(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    MPI_Allgatherv_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, ierr);
}

_EXTERN_C_ void mpi_allgatherv_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    MPI_Allgatherv_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, ierr);
}

_EXTERN_C_ void mpi_allgatherv__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    MPI_Allgatherv_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Allreduce(void *arg_0, void *arg_1, int arg_2, MPI_Datatype arg_3, MPI_Op arg_4, MPI_Comm arg_5) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Allreduce(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Allreduce(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Allreduce_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Allreduce((void*)arg_0, (void*)arg_1, *arg_2, (MPI_Datatype)(*arg_3), (MPI_Op)(*arg_4), (MPI_Comm)(*arg_5));
#else /* MPI-2 safe call */
    return_val = MPI_Allreduce((void*)arg_0, (void*)arg_1, *arg_2, MPI_Type_f2c(*arg_3), MPI_Op_f2c(*arg_4), MPI_Comm_f2c(*arg_5));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_ALLREDUCE(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    MPI_Allreduce_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, ierr);
}

_EXTERN_C_ void mpi_allreduce(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    MPI_Allreduce_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, ierr);
}

_EXTERN_C_ void mpi_allreduce_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    MPI_Allreduce_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, ierr);
}

_EXTERN_C_ void mpi_allreduce__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    MPI_Allreduce_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
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

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Barrier_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
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

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Bcast(void *arg_0, int arg_1, MPI_Datatype arg_2, int arg_3, MPI_Comm arg_4) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Bcast(arg_0, arg_1, arg_2, arg_3, arg_4);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Bcast(arg_0, arg_1, arg_2, arg_3, arg_4);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Bcast_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Bcast((void*)arg_0, *arg_1, (MPI_Datatype)(*arg_2), *arg_3, (MPI_Comm)(*arg_4));
#else /* MPI-2 safe call */
    return_val = MPI_Bcast((void*)arg_0, *arg_1, MPI_Type_f2c(*arg_2), *arg_3, MPI_Comm_f2c(*arg_4));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_BCAST(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *ierr) { 
    MPI_Bcast_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, ierr);
}

_EXTERN_C_ void mpi_bcast(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *ierr) { 
    MPI_Bcast_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, ierr);
}

_EXTERN_C_ void mpi_bcast_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *ierr) { 
    MPI_Bcast_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, ierr);
}

_EXTERN_C_ void mpi_bcast__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *ierr) { 
    MPI_Bcast_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Gather(void *arg_0, int arg_1, MPI_Datatype arg_2, void *arg_3, int arg_4, MPI_Datatype arg_5, int arg_6, MPI_Comm arg_7) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Gather(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Gather(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Gather_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Gather((void*)arg_0, *arg_1, (MPI_Datatype)(*arg_2), (void*)arg_3, *arg_4, (MPI_Datatype)(*arg_5), *arg_6, (MPI_Comm)(*arg_7));
#else /* MPI-2 safe call */
    return_val = MPI_Gather((void*)arg_0, *arg_1, MPI_Type_f2c(*arg_2), (void*)arg_3, *arg_4, MPI_Type_f2c(*arg_5), *arg_6, MPI_Comm_f2c(*arg_7));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_GATHER(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    MPI_Gather_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, ierr);
}

_EXTERN_C_ void mpi_gather(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    MPI_Gather_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, ierr);
}

_EXTERN_C_ void mpi_gather_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    MPI_Gather_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, ierr);
}

_EXTERN_C_ void mpi_gather__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    MPI_Gather_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Gatherv(void *arg_0, int arg_1, MPI_Datatype arg_2, void *arg_3, int *arg_4, int *arg_5, MPI_Datatype arg_6, int arg_7, MPI_Comm arg_8) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Gatherv(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, arg_8);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Gatherv(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, arg_8);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Gatherv_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *arg_8, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Gatherv((void*)arg_0, *arg_1, (MPI_Datatype)(*arg_2), (void*)arg_3, (int*)arg_4, (int*)arg_5, (MPI_Datatype)(*arg_6), *arg_7, (MPI_Comm)(*arg_8));
#else /* MPI-2 safe call */
    return_val = MPI_Gatherv((void*)arg_0, *arg_1, MPI_Type_f2c(*arg_2), (void*)arg_3, (int*)arg_4, (int*)arg_5, MPI_Type_f2c(*arg_6), *arg_7, MPI_Comm_f2c(*arg_8));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_GATHERV(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *arg_8, MPI_Fint *ierr) { 
    MPI_Gatherv_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, arg_8, ierr);
}

_EXTERN_C_ void mpi_gatherv(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *arg_8, MPI_Fint *ierr) { 
    MPI_Gatherv_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, arg_8, ierr);
}

_EXTERN_C_ void mpi_gatherv_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *arg_8, MPI_Fint *ierr) { 
    MPI_Gatherv_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, arg_8, ierr);
}

_EXTERN_C_ void mpi_gatherv__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *arg_8, MPI_Fint *ierr) { 
    MPI_Gatherv_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, arg_8, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Reduce(void *arg_0, void *arg_1, int arg_2, MPI_Datatype arg_3, MPI_Op arg_4, int arg_5, MPI_Comm arg_6) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Reduce(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Reduce(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Reduce_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Reduce((void*)arg_0, (void*)arg_1, *arg_2, (MPI_Datatype)(*arg_3), (MPI_Op)(*arg_4), *arg_5, (MPI_Comm)(*arg_6));
#else /* MPI-2 safe call */
    return_val = MPI_Reduce((void*)arg_0, (void*)arg_1, *arg_2, MPI_Type_f2c(*arg_3), MPI_Op_f2c(*arg_4), *arg_5, MPI_Comm_f2c(*arg_6));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_REDUCE(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    MPI_Reduce_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, ierr);
}

_EXTERN_C_ void mpi_reduce(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    MPI_Reduce_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, ierr);
}

_EXTERN_C_ void mpi_reduce_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    MPI_Reduce_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, ierr);
}

_EXTERN_C_ void mpi_reduce__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *ierr) { 
    MPI_Reduce_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Reduce_scatter(void *arg_0, void *arg_1, int *arg_2, MPI_Datatype arg_3, MPI_Op arg_4, MPI_Comm arg_5) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Reduce_scatter(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Reduce_scatter(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Reduce_scatter_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Reduce_scatter((void*)arg_0, (void*)arg_1, (int*)arg_2, (MPI_Datatype)(*arg_3), (MPI_Op)(*arg_4), (MPI_Comm)(*arg_5));
#else /* MPI-2 safe call */
    return_val = MPI_Reduce_scatter((void*)arg_0, (void*)arg_1, (int*)arg_2, MPI_Type_f2c(*arg_3), MPI_Op_f2c(*arg_4), MPI_Comm_f2c(*arg_5));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_REDUCE_SCATTER(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    MPI_Reduce_scatter_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, ierr);
}

_EXTERN_C_ void mpi_reduce_scatter(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    MPI_Reduce_scatter_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, ierr);
}

_EXTERN_C_ void mpi_reduce_scatter_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    MPI_Reduce_scatter_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, ierr);
}

_EXTERN_C_ void mpi_reduce_scatter__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    MPI_Reduce_scatter_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Scan(void *arg_0, void *arg_1, int arg_2, MPI_Datatype arg_3, MPI_Op arg_4, MPI_Comm arg_5) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Scan(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Scan(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Scan_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Scan((void*)arg_0, (void*)arg_1, *arg_2, (MPI_Datatype)(*arg_3), (MPI_Op)(*arg_4), (MPI_Comm)(*arg_5));
#else /* MPI-2 safe call */
    return_val = MPI_Scan((void*)arg_0, (void*)arg_1, *arg_2, MPI_Type_f2c(*arg_3), MPI_Op_f2c(*arg_4), MPI_Comm_f2c(*arg_5));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_SCAN(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    MPI_Scan_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, ierr);
}

_EXTERN_C_ void mpi_scan(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    MPI_Scan_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, ierr);
}

_EXTERN_C_ void mpi_scan_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    MPI_Scan_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, ierr);
}

_EXTERN_C_ void mpi_scan__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *ierr) { 
    MPI_Scan_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Scatter(void *arg_0, int arg_1, MPI_Datatype arg_2, void *arg_3, int arg_4, MPI_Datatype arg_5, int arg_6, MPI_Comm arg_7) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Scatter(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Scatter(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Scatter_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Scatter((void*)arg_0, *arg_1, (MPI_Datatype)(*arg_2), (void*)arg_3, *arg_4, (MPI_Datatype)(*arg_5), *arg_6, (MPI_Comm)(*arg_7));
#else /* MPI-2 safe call */
    return_val = MPI_Scatter((void*)arg_0, *arg_1, MPI_Type_f2c(*arg_2), (void*)arg_3, *arg_4, MPI_Type_f2c(*arg_5), *arg_6, MPI_Comm_f2c(*arg_7));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_SCATTER(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    MPI_Scatter_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, ierr);
}

_EXTERN_C_ void mpi_scatter(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    MPI_Scatter_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, ierr);
}

_EXTERN_C_ void mpi_scatter_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    MPI_Scatter_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, ierr);
}

_EXTERN_C_ void mpi_scatter__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *ierr) { 
    MPI_Scatter_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Scatterv(void *arg_0, int *arg_1, int *arg_2, MPI_Datatype arg_3, void *arg_4, int arg_5, MPI_Datatype arg_6, int arg_7, MPI_Comm arg_8) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Scatterv(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, arg_8);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Scatterv(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, arg_8);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Scatterv_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *arg_8, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Scatterv((void*)arg_0, (int*)arg_1, (int*)arg_2, (MPI_Datatype)(*arg_3), (void*)arg_4, *arg_5, (MPI_Datatype)(*arg_6), *arg_7, (MPI_Comm)(*arg_8));
#else /* MPI-2 safe call */
    return_val = MPI_Scatterv((void*)arg_0, (int*)arg_1, (int*)arg_2, MPI_Type_f2c(*arg_3), (void*)arg_4, *arg_5, MPI_Type_f2c(*arg_6), *arg_7, MPI_Comm_f2c(*arg_8));
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_SCATTERV(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *arg_8, MPI_Fint *ierr) { 
    MPI_Scatterv_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, arg_8, ierr);
}

_EXTERN_C_ void mpi_scatterv(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *arg_8, MPI_Fint *ierr) { 
    MPI_Scatterv_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, arg_8, ierr);
}

_EXTERN_C_ void mpi_scatterv_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *arg_8, MPI_Fint *ierr) { 
    MPI_Scatterv_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, arg_8, ierr);
}

_EXTERN_C_ void mpi_scatterv__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *arg_4, MPI_Fint *arg_5, MPI_Fint *arg_6, MPI_Fint *arg_7, MPI_Fint *arg_8, MPI_Fint *ierr) { 
    MPI_Scatterv_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6, arg_7, arg_8, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Waitall(int arg_0, MPI_Request *arg_1, MPI_Status *arg_2) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Waitall(arg_0, arg_1, arg_2);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Waitall(arg_0, arg_1, arg_2);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Waitall_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Waitall(*arg_0, (MPI_Request*)arg_1, (MPI_Status*)arg_2);
#else /* MPI-2 safe call */
    MPI_Request* temp_arg_1;
    MPI_Status* temp_arg_2;
    int i;
    temp_arg_1 = (MPI_Request*)malloc(sizeof(MPI_Request) * *arg_0);
    for (i=0; i < *arg_0; i++)
        temp_arg_1[i] = MPI_Request_f2c(arg_1[i]);
    temp_arg_2 = (MPI_Status*)malloc(sizeof(MPI_Status) * *arg_0);
    for (i=0; i < *arg_0; i++)
        MPI_Status_f2c(&arg_2[i], &temp_arg_2[i]);
    return_val = MPI_Waitall(*arg_0, temp_arg_1, temp_arg_2);
    for (i=0; i < *arg_0; i++)
        arg_1[i] = MPI_Request_c2f(temp_arg_1[i]);
    free(temp_arg_1);
    for (i=0; i < *arg_0; i++)
        MPI_Status_c2f(&temp_arg_2[i], &arg_2[i]);
    free(temp_arg_2);
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_WAITALL(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *ierr) { 
    MPI_Waitall_fortran_wrapper(arg_0, arg_1, arg_2, ierr);
}

_EXTERN_C_ void mpi_waitall(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *ierr) { 
    MPI_Waitall_fortran_wrapper(arg_0, arg_1, arg_2, ierr);
}

_EXTERN_C_ void mpi_waitall_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *ierr) { 
    MPI_Waitall_fortran_wrapper(arg_0, arg_1, arg_2, ierr);
}

_EXTERN_C_ void mpi_waitall__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *ierr) { 
    MPI_Waitall_fortran_wrapper(arg_0, arg_1, arg_2, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
_EXTERN_C_ int MPI_Waitany(int arg_0, MPI_Request *arg_1, int *arg_2, MPI_Status *arg_3) { 
    int return_val = 0;
    if (in_wrapper) return PMPI_Waitany(arg_0, arg_1, arg_2, arg_3);
    in_wrapper = 1;

#ifdef PMPI_EFFORT
    effort_do_stackwalk();
#endif
    effort_enter_comm();
    return_val = PMPI_Waitany(arg_0, arg_1, arg_2, arg_3);
    effort_exit_comm();
    in_wrapper = 0;
    return return_val;
}

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Waitany_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    return_val = MPI_Waitany(*arg_0, (MPI_Request*)arg_1, (int*)arg_2, (MPI_Status*)arg_3);
#else /* MPI-2 safe call */
    MPI_Request* temp_arg_1;
    MPI_Status temp_arg_3;
    int i;
    temp_arg_1 = (MPI_Request*)malloc(sizeof(MPI_Request) * *arg_0);
    for (i=0; i < *arg_0; i++)
        temp_arg_1[i] = MPI_Request_f2c(arg_1[i]);
    MPI_Status_f2c(arg_3, &temp_arg_3);
    return_val = MPI_Waitany(*arg_0, temp_arg_1, (int*)arg_2, &temp_arg_3);
    for (i=0; i < *arg_0; i++)
        arg_1[i] = MPI_Request_c2f(temp_arg_1[i]);
    free(temp_arg_1);
    MPI_Status_c2f(&temp_arg_3, arg_3);
#endif /* MPICH test */
    *ierr = return_val;
}

_EXTERN_C_ void MPI_WAITANY(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *ierr) { 
    MPI_Waitany_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, ierr);
}

_EXTERN_C_ void mpi_waitany(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *ierr) { 
    MPI_Waitany_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, ierr);
}

_EXTERN_C_ void mpi_waitany_(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *ierr) { 
    MPI_Waitany_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, ierr);
}

_EXTERN_C_ void mpi_waitany__(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *arg_2, MPI_Fint *arg_3, MPI_Fint *ierr) { 
    MPI_Waitany_fortran_wrapper(arg_0, arg_1, arg_2, arg_3, ierr);
}

/* ================= End Wrappers for MPI_File_sync ================= */


/* ================== C Wrappers for MPI_File_sync ================== */
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

/* =============== Fortran Wrappers for MPI_File_sync =============== */
static void MPI_Wait_fortran_wrapper(MPI_Fint *arg_0, MPI_Fint *arg_1, MPI_Fint *ierr) { 
    int return_val = 0;
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
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

/* ================= End Wrappers for MPI_File_sync ================= */



