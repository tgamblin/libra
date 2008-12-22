#ifndef PNMPI_MODULE_CALLPATH_H
#define PNMPI_MODULE_CALLPATH_H

#include <mpi.h>

/// This is the name of the stackwalk module, to be requested
/// By PNMPI modules that want to use stackwalks.
#define PNMPI_MODULE_CALLPATH "pnmpi-module-callpath"

/// This is the name to request to get at the callpath global.
/// Request via PNMPI with signature "p" to get a void**, 
/// cast to void*, and it points to a Callpath.
#define PNMPI_GLOBAL_CALLPATH "global-callpath"


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

  /// This is for internal use: it's called by the wrappers
  void set_callpath();

  /// This is a hook called by PnMPI at registration time.
  int PNMPI_RegistrationPoint();

  /// Does some overriding of signal handlers so that the MPI
  /// implementation doesn't hijack them.
  int MPI_Init(int *argc, char ***argv);

  // Following function(s) are NOT auto-generated as wrappers and
  // have special definitions in stackwalk_module.C
  int MPI_Finalize();

#ifdef __cplusplus
} // extern "C"
#endif //__cplusplus

#endif // PNMPI_MODULE_CALLPATH_H
