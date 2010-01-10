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
#include "io_utils.h"

using namespace effort;
using namespace cluster;
using namespace std;
using boost::numeric::ublas::matrix;
using wavelet::exists;

void usage() {
  cerr << "Usage: par-signature-cluster-test [-htvi] [-l level] [-n trace-length] [-s sigs-per-process] [-c clusters]" << endl;
  cerr << "  Parallel test case for clustering effort signatures." << endl;
  cerr << "Options:" << endl;
  cerr << "  -h         Show this message." << endl;
  cerr << "  -t         Output timing info to file." << endl;
  cerr << "  -v         Validate with sequential clustering and output Mirkin distance." << endl;
  cerr << "  -i         Initial sample size (before 2*k) for clara." << endl;
  cerr << "               Default is 40." << endl;
  cerr << "  -r         Max trial reps per k in clara." << endl;
  cerr << "               Default is 5." << endl;
  cerr << "  -l         Level of wavelet transform applied to signatures.  " << endl;
  cerr << "               Default is deep enough to make a 16-element signature." << endl;
  cerr << "  -n         Length of synthetic trace used to generate signatures." << endl;
  cerr << "               Default is 64." << endl;
  cerr << "  -s         Number of signatures generated per process." << endl;
  cerr << "               Default is 1." << endl;
  cerr << "  -k         Max number of clusters to search for." << endl;
  cerr << "               Default is 10." << endl;
  exit(1);
}

bool timing = false;
bool validate = false;
int level = -1;
size_t init_size = 40;
size_t max_reps = 5;
size_t trace_length = 64;
size_t sigs_per_process = 1;
size_t num_clusters = 10;

const size_t iterations = 10;   // num trials to account for variance in clara.

/// Uses getopt to read in arguments.
void get_args(int *argc, char ***argv, int rank) {
  int c;
  char *err;

  while ((c = getopt(*argc, *argv, "htvi:r:l:n:s:k:")) != -1) {
    switch (c) {
    case 'h':
      if (rank == 0) usage();
      exit(1);
      break;
    case 't':
      timing = true;
      break;
    case 'v':
      validate = true;
      break;
    case 'i':
      init_size = strtol(optarg, &err, 0);
      if (*err) usage();
      break;
    case 'r':
      max_reps = strtol(optarg, &err, 0);
      if (*err) usage();
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
    case 'k':
      num_clusters = strtol(optarg, &err, 0);
      if (*err) usage();
      break;
    default:
      if (rank == 0) usage();
      exit(1);
      break;
    }
  }

  // adjust params
  *argc -= optind;
  *argv += optind;
}



int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  get_args(&argc, &argv, rank);

  // make sure num_clusters is valid.
  num_clusters = min(num_clusters, size*sigs_per_process);
  
  uint32_t seed = 0;
  struct timeval time;
  gettimeofday(&time, 0);
  seed = time.tv_sec * time.tv_usec;

  MTRand rand(seed + rank);  // Seed signatures differently on each rank.
  matrix<double> data(sigs_per_process, trace_length);     // vector of local "traces"

  double noise = 1.0;
  for (size_t r=0; r < data.size1(); r++) {
    size_t type = (rank + r) % 6;

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
      case 3:
        data(r,c) = 2 * c;
        break;
      case 4:
        data(r,c) = 20 * cos(10*c)*sin(c/2.0);
        break;
      case 5:
        data(r,c) = 8;
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

  double total_mirkin = 0;
  par_kmedoids parkm;
  parkm.set_init_size(init_size);
  parkm.set_max_reps(max_reps);

  double total = 0.0;
  for (size_t iter=0; iter < iterations; iter++) {
    // time the clustering itself
    long long start = get_time_ns();
    parkm.clara(sigs, sig_euclidean_distance(), num_clusters);
    total += get_time_ns() - start;

    // gather distributed clusteirng and compare to exhaustive sequential version.
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
          all_sigs.push_back(effort_signature(&full_data(i,0), full_data.size2(), 0));
        }
        
        dissimilarity_matrix distance;
        build_dissimilarity_matrix(all_sigs, sig_euclidean_distance(), distance);
        
        kmedoids km;
        km.pam(distance, num_clusters);
        cout << endl;
        cout << "Seq k:   " << km.num_clusters() << endl;
        cout << km << endl;

        cout << endl;
        cout << "Par k:   " << parallel.num_clusters() << endl;
        cout << parallel << endl;
        cout << endl;

        double mirkin = mirkin_distance(parallel, km);
        total_mirkin += mirkin;
        cout << "Mirkin Distance:       " << mirkin << endl;
      }
    }
  }

  double avg = total / iterations;
  if (rank == 0) {
    cout << size << " processes" << endl;
    cout << "TOTAL:   " << total / 1e9 << endl;
    cout << "AVERAGE: " << avg   / 1e9 << endl;

    // output timing to a file
    if (timing) {
      ostringstream timing_filename;
      timing_filename << "times-" << size;
      size_t i=1;
      while (exists(timing_filename.str().c_str())) {
        timing_filename.str("");
        timing_filename << "times-" << size << "." << i;
        i++;
      }

      ofstream timing(timing_filename.str().c_str());
      parkm.get_timer().write(timing);
    }
  }

  if (validate && rank == 0) {
    cout << endl;
    cout << "Average Mirkin Distance: " << total_mirkin / iterations << endl;
  }
}
