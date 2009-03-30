#include <cstdlib>
#include <iostream>
using namespace std;

#include "wavelet.h"
using namespace wavelet;

#include "effort_dataset.h"
using namespace effort;


void usage() {
  cerr << "Usage: dataset_test <directory> <level>" << endl;
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 3) usage();
  
  string dirname(argv[1]);
  char *err;
  int level = strtol(argv[2], &err, 0);
  if (*err) usage();

  effort_dataset dataset(dirname, level);

  vector<proc_data*> procs;
  dataset.transpose(procs);

  cerr << procs.size() << endl;

  return 0;
}
