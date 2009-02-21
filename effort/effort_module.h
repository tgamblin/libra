#ifndef EFFORT_MODULE_H
#define EFFORT_MODULE_H

#include <mpi.h>
#include "effort_api.h"

/// Name of this module to be requested by users.
extern const char *const PNMPI_MODULE_EFFORT;

#ifdef __cplusplus
#include <string>

  /// Used by compressor to generate metadata.
  std::string id_to_metric_name(int id);

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
  void effort_init();

  /// To be called from MPI_Pcontrol.
  void effort_pcontrol(int level);

  /// To be called just before MPI is finalized.
  void effort_finalize();
  
  /// This is defined if we are using PNMPI_EFFORT (no PnMPI)
  void effort_do_stackwalk();

#ifdef __cplusplus
}
#endif //__cplusplus


#endif // EFFORT_MODULE_H
