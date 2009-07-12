#include "s3d_topology.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
using namespace std;

void usage() {
  cerr << "Usage: s3d_topo_test npx npy npz" << endl;
  exit(1);
}

int main(int argc, char **argv) {
  if (argc < 4) usage();

  char *err;
  int npx = strtol(argv[1], &err, 0);
  if (*err) usage();

  int npy = strtol(argv[2], &err, 0);
  if (*err) usage();

  int npz = strtol(argv[3], &err, 0);
  if (*err) usage();

  int i=0;
  for (int z=0; z < npz; z++) {
    for (int y=0; y < npy; y++) {
      for (int x=0; x < npx; x++) {
        cout << setw(8) << s3d::topo_rank(i++, npx, npy, npz);
      }
      cout << endl;
    }
    cout << endl;
  }
}
