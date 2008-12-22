#include <fstream>
#include <cstring>
using namespace std;

#include "wavelet.h"
#include "wt_lift.h"
#include "wt_utils.h"
#include "matrix_utils.h"
#include "ezw_encoder.h"
#include "ezw_decoder.h"
using wavelet::wt_matrix;
using namespace wavelet;


static const char *FILENAME = "ezw.out";


int main(int argc, char **argv) {
  bool pass = true;
  bool verbose = false;
  for (int i=1; i < argc; i++) {
    if (!strcmp(argv[i], "-v")) verbose = true;
  }
  
  ezw_encoder encoder;
  if (set_ezw_args(encoder, &argc, &argv)) {
    ezw_usage("ezwtest");
  }

  wt_lift lift;
  ezw_decoder decoder;
  
  int start = 2;
  int end = 10;

  int count = 0;
  double err_sum = 0;
  double ratio_sum = 0;

  //int r=9, c=9;
  for (int r=start; r < end; r++) {
    for (int c=start; c < end; c++) {
      int rows = 1 << r;
      int cols = 1 << c;
      
      wt_matrix mat(rows, cols);
      
      // fill matrix in with some randomly generated, cubic-ish values.
      srand(100);
      for (size_t i=0; i < mat.size1(); i++) {
        for (size_t j=0; j < mat.size2(); j++) {
          mat(i,j) = ((rand()/(double)RAND_MAX)+i+0.4*i*i-0.02*i*i*j);
        }
      }

      // transform values to get real wavelet coefficients
      wt_matrix trans = mat;

      int level = lift.fwt_2d(trans);

      // quantify the matrix here first, so that the coding will be exact.
      // Use a large scale factor to get fairly realistic numbers.
      for (size_t i=0; i < mat.size1(); i++) {
        for (size_t j=0; j < mat.size2(); j++) {
          trans(i,j) = (long long)(trans(i,j) * 1000);
        }
      }

      // write out ezw code to a file
      ofstream out(FILENAME);
      int size = encoder.encode(trans, out, level);
      out.close();

      // read in same file and deocde
      ifstream in(FILENAME);
      wt_matrix decoded;
      level = decoder.decode(in, decoded);

      // check that we get out what we put in.
      double nerr = nrmse(trans, decoded);
      double PSNR = psnr(trans, decoded);
      double ratio = (double)(rows * cols * sizeof(double))/size;

      if (nerr > 0) {
        pass = false;
      }

      if (verbose) {
        cout << "Normalized RMSE " << rows << " x " << cols << ":  \t" ;
        cout << setw(8) << nerr << "\t"
             << setw(8) << PSNR
             << "   ("  << ratio << ":1)";
        cout << endl;
      }

      count++;
      err_sum += nerr;
      ratio_sum += ratio;
    } 
  }
  
  if (verbose) {
    cout << endl;
    cout << "Mean Normalized RMSE:  \t" << setw(10) << err_sum/count << endl;
    cout << "Mean Compression Ratio:\t" << setw(8) << ratio_sum/count << ":1" << endl;
    cout << (pass ? "PASSED" : "FAILED") << endl;
  }

  exit(pass ? 0 : 1);
}
