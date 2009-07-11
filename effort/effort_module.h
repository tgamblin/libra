#ifndef EFFORT_MODULE_H
#define EFFORT_MODULE_H

#include <mpi.h>
#include "effort_api.h"

/// Name of this module to be requested by users.
extern const char *const PNMPI_MODULE_EFFORT;

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

  /// Registration point for PNMPI module -- inits everything
  /// and requests service modules that this one uses.
  int PNMPI_RegistrationPoint();
  
  
  /// Called by wrappers to enter comm region
  void effort_enter_comm();


  /// Called by wrappers to leave comm region
  void effort_exit_comm();
  

  // --- below are MPI-related routines --- //

  /// To be called before MPI is inited
  void effort_preinit();

  /// To be called after MPI is inited but before other MPI calls
  void effort_postinit();

  /// To be called from MPI_Pcontrol.
  void effort_pcontrol(int level);

  /// To be called just before MPI is finalized.
  void effort_finalize();
  
  /// This is defined if we are using PNMPI_EFFORT (no PnMPI)
  void effort_do_stackwalk();

  /// temporary addition for topo experiment
  void effort_set_dims(size_t x, size_t y, size_t z);
  

#ifdef __cplusplus
}
#endif //__cplusplus


#endif // EFFORT_MODULE_H
