#include <sys/time.h>
#include <cstring>
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
using wavelet::wt_matrix;
using namespace wavelet;

static const char *PAR_FILENAME = "parezw.out";
static const char *SEQ_FILENAME = "seqezw.out";

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

  par_ezw_encoder par_encoder;
  par_encoder.set_encoding_type(HUFFMAN);
  if (set_ezw_args(par_encoder, &argc, &argv)) {
    ezw_usage("parezwtest");
  }

  ezw_encoder encoder;
  encoder.set_pass_limit(par_encoder.get_pass_limit());
  encoder.set_encoding_type(par_encoder.get_encoding_type());
  encoder.set_scale(par_encoder.get_scale());

  wt_parallel pwt;          // parallel and local transformers
  wt_direct dwt;

  wt_matrix mat(128, 128);  // initially distributed matrix

  // level starts at max possible for matrix dimensions, then we
  // set it explicitly and do transforms at sublevels, too.
  for (int level = -1; level != 0; level--) {
    // initialize matrix
    for (size_t i=0; i < mat.size1(); i++) {
      for (size_t j=0; j < mat.size2(); j++) {
        mat(i,j) = ((.06 + rank) * (5+i+0.4*i*i-0.02*i*i*j));
      }
    }

    // do parallel transform on all data, record remote level
    level = pwt.fwt_2d(mat, level);

    // quantify the matrix here first, so that the coding will be exact.
    // Use a large scale factor to get fairly realistic numbers.
    for (size_t i=0; i < mat.size1(); i++) {
      for (size_t j=0; j < mat.size2(); j++) {
        mat(i,j) = (long long)(mat(i,j) * 1000);
      }
    }

    // find the root of the reduction the encoder will do
    // this is where the data will be when we're done.
    int root = par_encoder.get_root(MPI_COMM_WORLD);

    ofstream par_output;
    if (rank == root) par_output.open(PAR_FILENAME);
    size_t par_bytes = par_encoder.encode(mat, par_output, level, MPI_COMM_WORLD);
    if (rank == root) par_output.close();

    // gather the parallel-transformed data and sequentially code it.
    wt_matrix par_fwt;
    wt_parallel::gather(par_fwt, mat, MPI_COMM_WORLD, root);


    if (rank == root) {
      wt_parallel::reassemble(par_fwt, size, level);
      
      // sequentially code reassembled distributed array
      ofstream seq_out(SEQ_FILENAME);
      size_t seq_bytes = encoder.encode(par_fwt, seq_out, level);

      seq_out.close();

      // decode parallel-coded data
      ezw_decoder decoder;

      wt_matrix par_decoded;
      ifstream par_file(PAR_FILENAME);
      decoder.decode(par_file, par_decoded);

      // decode sequentially-coded data
      wt_matrix seq_decoded;
      ifstream seq_file(SEQ_FILENAME);
      decoder.decode(seq_file, seq_decoded);

      // check to make sure dimensions match
      if (par_decoded.size1() != seq_decoded.size1() ||
          par_decoded.size2() != seq_decoded.size2()) {
        
        pass = false;
        if (verbose) {
          cout << "Decoded Sizes do not agree: " 
               << seq_decoded.size1() << "x" << seq_decoded.size2() << " vs "
               << par_decoded.size1() << "x" << par_decoded.size2()
               << endl;
        }

      } else {
        // If dimensions do match, report nrmse
      	double nerr = nrmse(seq_decoded, par_decoded);
        double serr = nrmse(par_fwt, seq_decoded);
        double perr = nrmse(par_fwt, par_decoded);

        if (nerr > 0 || serr > 0 || perr > 0) {
          pass = false;
        }

        if (verbose) {
          int rows = par_decoded.size1();
          int cols = par_decoded.size2();

          cout << "Normalized RMSE " << rows << " x " << cols << ":  \t" ;
          cout << setw(8) << nerr;
          cout << "  " << serr;
          cout << "  " << perr;
          cout << "  " << setw(10) << seq_bytes;
          cout << "  " << setw(10) << par_bytes;
          cout << endl;
        }
      }
    }
  }

  MPI_Finalize();

  if (verbose) {
    cout << (pass ? "PASSED" : "FAILED") << endl;
  }

  exit(pass ? 0 : 1);
}

