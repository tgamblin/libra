#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <sys/time.h>

#include "effort_signature.h"
#include "io_utils.h"
#include "wavelet.h"
#include "kmedoids.h"
#include "par_kmedoids.h"
#include "bic.h"
#include "matrix_utils.h"
#include "MersenneTwister.h"

using namespace effort;
using namespace cluster;
using namespace std;
using boost::numeric::ublas::matrix;

void usage() {
  cerr << "Usage: par-signature-cluster-test [-htv] [-l level] [-n trace-length] [-s sigs-per-process]" << endl;
  cerr << "  Parallel test case for clustering effort signatures." << endl;
  cerr << "Options:" << endl;
  cerr << "  -h         Show this message." << endl;
  cerr << "  -t         Output timing info to file." << endl;
  cerr << "  -v         Validate with sequential clustering and output Mirkin distance." << endl;
  cerr << "  -l         Level of wavelet transform applied to signatures.  " << endl;
  cerr << "               Default is deep enough to make a 16-element signature." << endl;
  cerr << "  -n         Length of synthetic trace used to generate signatures." << endl;
  cerr << "               Default is 64." << endl;
  cerr << "  -s         Number of signatures generated per process." << endl;
  cerr << "               Default is 1." << endl;
  cerr << "  -c         Max number of clusters to search for." << endl;
  cerr << "               Default is 1." << endl;
  exit(1);
}

bool timing = false;
bool validate = false;
int level = -1;
size_t trace_length = 64;
size_t sigs_per_process = 1;
size_t max_clusters = 10;

/// Uses getopt to read in arguments.
void get_args(int *argc, char ***argv) {
  int c;
  char *err;

  while ((c = getopt(*argc, *argv, "htvl:n:s:")) != -1) {
    switch (c) {
    case 'h':
      usage();
      break;
    case 't':
      timing = true;
      break;
    case 'v':
      validate = true;
      break;
    case 'l':
      level = strtol(optarg, &err, 0);
      if (*err) usage();
      break;
    case 'n':
      trace_length = strtol(optarg, &err, 0);
      if (*err) usage();
      break;
    case 's':
      sigs_per_process = strtol(optarg, &err, 0);
      if (*err) usage();
      break;
    default:
      usage();
      break;
    }
  }

  // adjust params
  *argc -= optind;
  *argv += optind;
}



int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  get_args(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  uint32_t seed = 0;
  if (rank == 0) {
    struct timeval time;
    gettimeofday(&time, 0);
    seed = time.tv_sec * time.tv_usec;
  }

  MTRand rand(seed + rank);  // Seed signatures differently on each rank.
  matrix<double> data(sigs_per_process, trace_length);     // vector of local "traces"

  double noise = 1.0;
  for (size_t r=0; r < data.size1(); r++) {
    size_t type = (rank + r) % 3;

    for (size_t c=0; c < data.size2(); c++) {
      switch (type) {
      case 0:
        data(r,c) = 10 * sin(c/5.0);
        break;
      case 1:
        data(r,c) = sin(c/5.0);
        break;
      case 2:
        data(r,c) = 10 * sin(c/5.0)*sin(c/5.0) * cos(c);
        break;
      }
      
      // add in noise so that things aren't *exactly* the same.
      data(r,c) += noise * rand();
    }
  }

  vector<effort_signature> sigs(data.size1());
  for (size_t i=0; i < data.size1(); i++) {
    sigs[i] = effort_signature(&data(i,0), data.size2(), level);
  }


  par_kmedoids parkm;

  const size_t trials = 10;
  long long start = get_time_ns();

  for (size_t i=0; i < trials; i++) {
    parkm.xclara(sigs, sig_euclidean_distance(), max_clusters, trace_length);
  }
  double total = get_time_ns() - start;
  double avg = total / trials;
  
  if (timing && rank == 0) {
    ostringstream timing_filename;
    timing_filename << "times-" << size;
    ofstream timing(timing_filename.str().c_str());
    parkm.get_timer().write(timing);
    
    cout << size << " processes" << endl;
    cout << "TOTAL:   " << total / 1e9 << endl;
    cout << "AVERAGE: " << avg   / 1e9 << endl;
  }

  if (validate) {
    cluster::partition parallel;
    parkm.gather(parallel, 0);

    matrix<double> full_data(size * sigs_per_process, trace_length);
    MPI_Gather(&data(0,0),      sigs_per_process * trace_length, MPI_DOUBLE,
               &full_data(0,0), sigs_per_process * trace_length, MPI_DOUBLE,
               0, MPI_COMM_WORLD);
    
    if (rank == 0) {
      // compare parallel clustering with local clustering.
      vector<effort_signature> all_sigs;
      for (size_t i=0; i < full_data.size1(); i++) {
        all_sigs.push_back(effort_signature(&full_data(i,0), full_data.size2(), level));
      }

      dissimilarity_matrix distance;
      build_dissimilarity_matrix(all_sigs, sig_euclidean_distance(), distance);
      
      kmedoids km;
      double best_bic = km.xpam(distance, max_clusters, trace_length);
      cout << endl;
      cout << "Seq k:   " << km.medoid_ids.size() << endl;
      cout << "Seq BIC: " << best_bic << endl;
      cout << km << endl;
      cout << endl;
      cout << "Par k:   " << parallel.medoid_ids.size() << endl;
      cout << "Par BIC: " << bic(parallel, matrix_distance(distance), trace_length) << endl;
      cout << parallel << endl;
      cout << endl;
      cout << "Mirkin Distance: " << mirkin_distance(parallel, km) << endl;
      cout << "Mirkin Distance: " << mirkin_distance(km, parallel) << endl;
    }
  }
}
