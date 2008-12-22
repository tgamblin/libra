#include <iostream>
#include <fstream>
using namespace std;

#include "wavelet.h"
#include "wt_direct.h"
#include "ezw.h"
#include "ezw_decoder.h"
#include "matrix_utils.h"
using namespace wavelet;

#include "effort_metadata.h"
using namespace effort;


void usage() {
  cerr << "Usage: nrmse m1 [m2]" << endl;
  cerr << "  Calculates normalized root mean squared error between m1 and m2." << endl;
  cerr << "  Also outputs PSNR and similarity (symmetric NRMSE)." << endl;
  cerr << "  If m2 is not provided, tries to find it in the exact/ subfolder, if it exists." << endl;
  exit(1);
}


int main(int argc, char **argv) {
  if (argc < 2 || argc > 3) {
    usage();
  }

  string compressed_filename(argv[1]);

  wavelet::wt_matrix exact;

  if (argc > 2) {
    string exact_filename(argv[2]);
    ifstream exact_file(exact_filename.c_str());

    effort_metadata md;
    ezw_decoder decoder;
    wt_direct wt;

    effort_metadata::read_in(exact_file, md);
    int level = decoder.decode(exact_file, exact);
    wt.iwt_2d(exact, level);
    
  } else {
    string metric;
    int type, number;
    if (!parse_filename(compressed_filename, &metric, &type, &number)) {
      cerr << "Can't make sense of filename: " << compressed_filename << endl;
    }

    ostringstream exact_str;
    exact_str << "exact/exact-" << metric << "-" << type << "-" << number;

    string exact_filename = exact_str.str();
    if (!read_matrix(exact_filename.c_str(), exact)) {
      cerr << "Couldn't open file: '" << exact_filename << "'" << endl;
      exit(1);
    }
 }


  wavelet::wt_matrix reconstruction;
  ifstream comp_file(compressed_filename.c_str());
  if (comp_file.fail()) {
    cerr << "Couldn't open file: '" << compressed_filename << "'" << endl;
    exit(1);
  }

  effort_metadata md;
  ezw_decoder decoder;
  wt_direct wt;

  effort_metadata::read_in(comp_file, md);
  int level = decoder.decode(comp_file, reconstruction);
  wt.iwt_2d(reconstruction, level);

  // output error to the metadata file if we're verifying.
  cout << "NRMSE:\t" << nrmse(exact, reconstruction) << endl;
  cout << "PSNR:\t" << psnr(exact, reconstruction) << endl;
  cout << "SIMIL:\t" << similarity(exact, reconstruction) << endl;
}
