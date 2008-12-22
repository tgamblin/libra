#include <mpi.h>

#include <fstream>
#include <sstream>
using namespace std;

#include "wavelet.h"
#include "matrix_utils.h"
#include "effort_api.h"
using namespace wavelet;

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  // read in file with bunny image.
  wt_matrix bunny;
  read_matrix("bunny.dat", bunny);

  // get rank and size
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // figure out scaling factor from image size (128x128) to (size x size)
  double scale = ((double)bunny.size1() / size);
  double x = rank * scale;  
  for (size_t i = 0; i < size; i++) {
    double y = i * scale;
    double value = interp_bilinear(bunny, x, y);

    // record effort every timestep so as to make a 3d bunny 
    // with the heightmap values.
    record_effort(1, &value);
    progress_step();
  }

  MPI_Finalize();
}


