#include <mpi.h>
#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;

#include "wt_parallel.h"
#include "wt_direct.h"
#include "wt_utils.h"
#include "par_ezw_encoder.h"
#include "ezw_decoder.h"
#include "timing.h"
using wavelet::wt_matrix;
using namespace wavelet;

static const char *PAR_FILENAME = "parspeedtest.out";

double get_time_seconds() {
  return (get_time_ns() / 1e9);
}

/// This verifies that the parallel wavelet transform produces
/// exactly the same output as the convolving transform.
int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  par_ezw_encoder par_encoder;
  if (set_ezw_args(par_encoder, &argc, &argv)) {
    ezw_usage("parspeedtest");
  }

  ezw_decoder decoder;
  wt_parallel pwt;          // parallel and local transformers
  wt_direct dwt;

  double wt_total = 0;
  double ezw_total = 0;
  double start_time;

  wt_matrix mat(128, 128);  // initially distributed matrix
  const size_t count = 5;
  if (rank == 0) {
    cerr << "Average times for " << count << " trials of " 
         << mat.size1() << "x" << mat.size2() << " transforms." << endl;
  }

  long root = par_encoder.get_root(MPI_COMM_WORLD);

  // level starts at max possible for matrix dimensions, then we
  // set it explicitly and do transforms at sublevels, too.
  for (int level = -1; level != 0; level--) {
    // initialize matrix
    for (size_t i=0; i < mat.size1(); i++) {
      for (size_t j=0; j < mat.size2(); j++) {
        mat(i,j) = ((.06 + rank) * (5+i+0.4*i*i-0.02*i*i*j));
      }
    }

    // do parallel transform and coding on all data
    for (size_t i=0; i < count; i++) {
      start_time = get_time_seconds();
      level = pwt.fwt_2d(mat, level);
      wt_total += get_time_seconds() - start_time;

      ofstream par_output;
      if (rank == root) {
        par_output.open(PAR_FILENAME);
      }
      
      start_time = get_time_seconds();
      par_encoder.encode(mat, par_output, level, MPI_COMM_WORLD);
      ezw_total += get_time_seconds() - start_time;
    }
    if (rank == root) {
      cout << "Level " << level << "\twt: " << (wt_total / count) << "\tezw: " << (ezw_total / count) << endl;
    }
  }

  MPI_Finalize();
}

