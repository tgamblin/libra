#include "effort_module.h"

#include <mpi.h>

#include <sstream>
#include <fstream>
using namespace std;

#include "timing.h"

static double init_time;           /// Time MPI_Init ran.
static double finalize_time;       /// Time MPI_Finalize ran.

extern "C" int MPI_Init(int *argc, char ***argv) {
  init_time = get_time_ns();
  return PMPI_Init(argc, argv);
}


static void dump_timing() {
  int size;
  PMPI_Comm_size(MPI_COMM_WORLD, &size);
  
  // print out all the callpaths for each process
  ostringstream fn;
  fn << "times-" << size;
  ofstream times(fn.str().c_str());
  
  times << "APP:\t"   << finalize_time - init_time << endl;
  times << "TOTAL:\t" << finalize_time - init_time << endl;
}


extern "C" int MPI_Finalize() {
  int rank;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  finalize_time = get_time_ns();

  // dump times on rank 0.
  if (rank == 0) {
    dump_timing();
  }

  return PMPI_Finalize();
}
