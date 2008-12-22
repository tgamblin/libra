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
  

  /// MPI_Pcontrol is needed to mark progress steps and
  /// effort phase boundaries.
  int MPI_Pcontrol(const int level, ...);

  
  /// We hijack this so we can start timing in MPI_Init().
  int MPI_Init(int *argc, char ***argv);


  /// We wrap MPI_Finalize() here to compress all recorded effort
  /// data and write it out to disk.
  int MPI_Finalize();
 
  /// This is defined if we are using PNMPI_EFFORT (no PnMPI)
  void effort_do_stackwalk();

#ifdef __cplusplus
}
#endif //__cplusplus


#endif // EFFORT_MODULE_H
