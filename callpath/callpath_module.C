#include "callpath_module.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <set>
#include <algorithm>
using namespace std;

#include "pnmpimod.h"
#include "mpi_utils.h"
#include "Callpath.h"
#include "CallpathRuntime.h"

// Runtime support for callpath stuff.
static CallpathRuntime runtime;

// Global callpath variable; initially null
static Callpath callpath;


void set_callpath() {
  callpath = runtime.doStackwalk(2);
}


#define CHECK_CALL(fn_call) \
   if ((err = (fn_call)) != PNMPI_SUCCESS) return MPI_ERROR_PNMPI


int PNMPI_RegistrationPoint() {
  int err = PNMPI_SUCCESS;

#ifndef PMPI_EFFORT
  CHECK_CALL(PNMPI_Service_RegisterModule(PNMPI_MODULE_CALLPATH));
  
  PNMPI_Global_descriptor_t global;
  strcpy(global.name, PNMPI_GLOBAL_CALLPATH);
  global.addr.p = reinterpret_cast<void**>(&callpath);
  global.sig = 'p';
  CHECK_CALL(PNMPI_Service_RegisterGlobal(&global));
#endif
  
  return err;
}


int MPI_Finalize() {
  set_callpath();

  size_t num_walks = runtime.numWalks();
  size_t bad_walks = runtime.badWalks();

  size_t total_walks;
  size_t total_bad_walks;
  PMPI_Reduce(&num_walks, &total_walks, 1, MPI_SIZE_T, MPI_SUM, 0, MPI_COMM_WORLD);
  PMPI_Reduce(&bad_walks, &total_bad_walks, 1, MPI_SIZE_T, MPI_SUM, 0, MPI_COMM_WORLD);

  int rank;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    // Print out percent bad stackwalks here.
    cerr << total_bad_walks << " errors out of " << total_walks << " stackwalks." << endl;
    cerr << (100.0 * total_bad_walks / total_walks) << "% bad walks" << endl;
  }

  return PMPI_Finalize();
}
