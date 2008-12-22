#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <map>
using namespace std;

typedef map<int, size_t> histogram_t;
histogram_t counts;

int MPI_Init(int *argc, char ***argv) {
  int return_val = PMPI_Init(argc, argv);

  int rank;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    printf("=================================================\n");
    printf("=  Running with MPI_Pcontrol() counter module.  =\n");
    printf("=================================================\n");
  }

  return return_val;
}

int MPI_Pcontrol(const int type, ...) {
  counts[type]++;
  return 0;
}


int MPI_Finalize() {
  int rank;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  if (rank == 0) {
    cout << "Calls to MPI_Pcontrol(int)\n";
    cout << setw(5) << "TYPE" << setw(12) << "COUNT" << endl;
    for (histogram_t::iterator i=counts.begin(); i != counts.end(); i++) {
      cout << setw(5) << i->first << setw(12) << i->second << endl;
    }
  }
  
  return PMPI_Finalize();
}
