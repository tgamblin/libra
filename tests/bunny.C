#include <mpi.h>

#include <fstream>
#include <sstream>
#include <cstdlib>
using namespace std;

#include "wavelet.h"
#include "matrix_utils.h"
#include "effort_api.h"
using namespace wavelet;

#ifndef SRC_DIR
#define SRC_DIR ""
#endif

/// Try to find bunny.dat in gnu standard location, $(srcdir).
/// Expect this to be passed in as env var or via -DSRC_DIR=
/// at compile time.  Failing those, try the working directory.
void find_bunny(string& location) {
  ostringstream bunny_path;

  if (getenv("srcdir")) {
    bunny_path << getenv("src_dir");
  } else if (strlen(SRC_DIR)) {
    bunny_path << SRC_DIR;
  } else {
    bunny_path << ".";
  }
  bunny_path << "/bunny.dat";

  location = bunny_path.str();
}


/// Loads a height-map of the Stanford bunny from a file.  Outputs 
/// synthetic, bilinear-interpolated data from this file to a trace
/// using the effort API.  If the trace looks like a bunny, you know
/// this test worked.
int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // read in file with bunny image.
  wt_matrix bunny;

  string bunny_data;
  find_bunny(bunny_data);
  if (!read_matrix(bunny_data.c_str(), bunny)) {
    if (rank == 0) {
      cerr << "Can't find file '" << bunny_data << "'" << endl;
    }
    exit(1);
  }

  // figure out scaling factor from image size (128x128) to (size x size)
  double scale = ((double)bunny.size1() / size);
  double x = rank * scale;  

  const char* metrics[] = {"Bunny", "BunnyFlipped", "Bunny2x", "Bunny0.5x"};
  init_metrics(4, metrics);
  
  double values[4];
  for (int i = 0; i < size; i++) {
    double y = i * scale;

    // record effort every timestep so as to make a 3d bunny with
    // the heightmap values. Record transpose and scaled versions to 
    // test recording of multiple values at once.
    values[0] = interp_bilinear(bunny, x, y);
    values[1] = interp_bilinear(bunny, y, x);
    values[2] = 2.0 * values[0];
    values[3] = 0.5 * values[0];

    record_effort(values);
    progress_step();
  }

  MPI_Finalize();
}


