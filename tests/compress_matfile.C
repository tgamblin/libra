#include <fstream>
#include <cstdlib>
using namespace std;

#include "wavelet.h"
#include "wt_lift.h"
#include "wt_direct.h"
#include "wt_utils.h"
#include "matrix_utils.h"
#include "ezw_encoder.h"
#include "ezw_decoder.h"
using wavelet::wt_matrix;
using namespace wavelet;

static const char *PROGNAME = "compress_matfile";
static const char *FILENAME = "compress_matfile.out";


int main(int argc, char **argv) {
  ezw_encoder encoder;
  if (set_ezw_args(encoder, &argc, &argv)) {
    ezw_usage(PROGNAME, "file");
  }

  if (!argc) ezw_usage(PROGNAME, "file");

  int level = -1;
  wt_lift wt;
  ezw_decoder decoder;
  
  wt_matrix mat;
  if (!read_matrix(argv[1], mat)) {
    cerr << "No such file: " << argv[1] << endl;
    exit(1);
  }

  // transform values
  wt_matrix trans = mat;
  level = wt.fwt_2d(trans, level);

  // Do an inverse transform on the matrix and record the error just from iwt.
  wt_matrix iwt = trans;
  wt.iwt_2d(iwt, level);
  double wt_err = nrmse(mat, iwt);
  double wt_psnr = psnr(mat, iwt);

  // ezw-encode the transformed matrix
  ofstream encoded_out(FILENAME);
  long tb = encoder.encode(trans, encoded_out, level);
  cerr << tb << endl;
  encoded_out.close();
  
  // decode the ezw-coded file.
  ifstream encoded_in(FILENAME);
  wt_matrix unezw;
  decoder.decode(encoded_in, unezw);
  
  // figure out error from ezw coding.
  double ezw_err = nrmse(trans, unezw);
  double ezw_psnr = psnr(trans, unezw);

  // inverse-transform the decoded data and get error
  wt.iwt_2d(unezw, level);
  double wt_ezw_err = nrmse(mat, unezw);
  double wt_ezw_psnr = psnr(mat, unezw);

  cout << mat.size1() << " x " << mat.size2() << "\t     NRMSE      PSNR" << endl;
  cout << "WT NRMSE:      " << setw(8) << wt_err     << "   " << wt_psnr     << endl;
  cout << "EZW NRMSE:     " << setw(8) << ezw_err    << "   " << ezw_psnr    << endl;
  cout << "WT_EZW NRMSE:  " << setw(8) << wt_ezw_err << "   " << wt_ezw_psnr << endl;
}
