#include <mpi.h>
#include <iostream>
#include <iomanip>
using namespace std;

#include "wt_parallel.h"
#include "wt_direct.h"
using wavelet::wt_matrix;
using namespace wavelet;

/// This verifies that the parallel wavelet transform produces
/// exactly the same output as the convolving transform.
int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  bool pass = true;
  bool verbose = false;
  for (int i=1; i < argc; i++) {
    if (!strcmp(argv[i], "-v")) verbose = true;
  }
  
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  wt_parallel pwt;          // parallel and local transformers
  wt_direct dwt;

  wt_matrix mat(128, 256);  // initially distributed matrix

  // level starts at max possible for matrix dimensions, then we
  // set it explicitly and do transforms at sublevels, too.
  for (int level = -1; level != 0; level--) {
    // initialize matrix
    for (size_t i=0; i < mat.size1(); i++) {
      for (size_t j=0; j < mat.size2(); j++) {
        mat(i,j) = ((.06 + rank) * (5+i+0.4*i*i-0.02*i*i*j));
      }
    }
    
    // collect plain matrix for sequential transform
    wt_matrix original;
    wt_parallel::gather(original, mat, MPI_COMM_WORLD);

    // do remote transform on all data, record remote level
    level = pwt.fwt_2d(mat, level);

    // do local transform at same level 
    wt_matrix localwt;
    if (rank == 0) {
      localwt = original;
      dwt.fwt_2d(localwt, level);
    }

    wt_matrix par_fwt;
    wt_parallel::gather(par_fwt, mat, MPI_COMM_WORLD);
    if (rank == 0) {
      wt_parallel::reassemble(par_fwt, size, level);

      double err = nrmse(localwt, par_fwt);
      if (err > 0) {
        pass = false;
      }

      if (verbose) {
        cout << "Level " << level << " NRMSE: " << setw(12) << err;
      }
    }

    // do remote inverse transform, compare to local inverse
    pwt.iwt_2d(mat, level);
    wt_matrix par_iwt;
    wt_parallel::gather(par_iwt, mat, MPI_COMM_WORLD);
    if (rank == 0) {
      dwt.iwt_2d(localwt, level);

      double err = nrmse(localwt, par_iwt);
      if (err > 0) {
        pass = false;
      }

      if (verbose) {
        cout << setw(12) << nrmse(localwt, par_iwt);
      }
    }
    
    if (verbose && rank == 0) cout << endl;
  }
  MPI_Finalize();
  
  if (verbose) {
    cout << (pass ? "PASSED" : "FAILED") << endl;
  }

  exit(pass ? 0 : 1);
}

