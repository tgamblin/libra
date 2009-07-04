#include <sys/stat.h>
#include <stdint.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
using namespace std;

#include "FrameId.h"
#include "ModuleId.h"
#include "Metric.h"
#include "Timer.h"
#include "io_utils.h"
#include "matrix_utils.h"
using namespace wavelet;

#include "effort_data.h"
#include "effort_dataset.h"
#include "effort_params.h"
#include "parallel_compressor.h"
#include "parallel_decompressor.h"
using namespace effort;

#include "kmedoids.h"
using namespace cluster;
#include "ltqnorm.h"

string dirname;              // effort directory
int level = -1;              // approximation level for data set
size_t pass_limit = 0;       // pass limit for data set
size_t frequency = 8;       // sample scheme update interval (in actual progress steps)
size_t strata = 1;           // # clusters
double confidence = 0.90;    // confidence % for sampling  (bt/w 0 and 1)
double error = 0.08;         // error bound for sampling (bt/w 0 and 1... probably closer to 0)
size_t iterations = 0;       // number of times to cluster.

size_t scale;                // scale factor calculated from approximation level.
int scale_levels;            // scale factor calculated from approximation level.


void usage() {
  cerr << "Usage: sample-test [options] dir" << endl;
  cerr << "Parameters:" << endl;
  cerr << "  dir         A directory full of effort files." << endl;
  cerr << "Options: " << endl;
  cerr << "  -h             Display this message and quit." << endl;
  cerr << "  -l level       Approximation level for data analysis.  Defaults to -1 (expand all)" << endl;
  cerr << "  -p pass_limit  Number of EZW passes to decompress.     Defaults to 0 (all passes)" << endl;
  cerr << "  -f freq        Update frequency (in progress steps)    Default is 8"     << endl;
  cerr << "  -s strata      Number of strata (per update).          Defaults to 1."  << endl;
  cerr << "  -c conf        Confidence bound in (0..1).             Default is 0.90" << endl;
  cerr << "  -e err         Error bound in (0..1).                  Default is 0.08" << endl;
  cerr << "  -i iterations  Number of times to cluster.             Default is 0 (all)" << endl;
  exit(1);
}

void get_args(int& argc, char**& argv) {
  int c;
  char *err;
  ifstream vdata;

  while ((c = getopt(argc, argv, "hl:p:f:s:c:e:i:")) != -1) {
    switch (c) {
    case 'h':
      usage();
      break;
    case 'l':
      level = strtol(optarg, &err, 0);
      if (*err) usage();
      break;
    case 'p':
      pass_limit = strtol(optarg, &err, 0);
      if (*err) usage();
      break;
    case 'f':
      frequency = strtol(optarg, &err, 0);
      if (*err) usage();
      break;
    case 's':
      strata = strtol(optarg, &err, 0);
      if (*err) usage();
      break;
    case 'c':
      confidence = strtod(optarg, &err);
      if (*err) usage();
      break;
    case 'e':
      error = strtod(optarg, &err);
      if (*err) usage();
      break;
    case 'i':
      iterations = strtol(optarg, &err, 0);
      if (*err) usage();
      break;
    default:
      usage();
      break;
    }
  }

  // adjust params
  argc -= optind;
  argv += optind;
}



struct distance_fun {
  size_t start_col, end_col;

  distance_fun(size_t start, size_t end) : start_col(start), end_col(end) {
    assert(start_col < end_col);
  }
  
  /// Calculates mean of all rows (mean over time per region)
  /// Then takes euclidean distance bt/w the vectors of row means.
  double operator()(proc_data* left, proc_data* right) {
    size_t rows = left->data.size1();
    size_t cols = end_col - start_col;

    double euclid_sum = 0;
    for (size_t i=0; i < rows; i++) {
      double lsum = 0;
      double rsum = 0;
      for (size_t j=start_col; j < end_col; j++) {
        lsum += left->data(i,j);
        rsum += right->data(i,j);
      }
      double diff = (lsum/cols) - (rsum/cols);
      euclid_sum += diff*diff;
    }
    return sqrt(euclid_sum);
  }
};


struct summary {
  double min;
  double max;
  double mean;
  double top_quartile;
  double bottom_quartile;
  
  summary(vector<double>& numbers) {
    if (numbers.size() == 0) {
      cerr << "Bad summary dataset!" << endl;
      exit (1);
    }

    sort(numbers.begin(), numbers.end());
    double sum = 0;
    for (size_t i=0; i < numbers.size(); i++) {
      sum  += numbers[i];
    }
    min = numbers[0] ? numbers[0] : 1;
    max = numbers[numbers.size() - 1];
    double quartile_size = numbers.size() / 4.0;
    top_quartile = numbers[(size_t)floor(numbers.size() - quartile_size)];
    bottom_quartile = numbers[(size_t)floor(quartile_size)];

    mean = sum / numbers.size();
  }
};

ostream& operator<<(ostream& out, const summary& ss) {
  const int w=13;
  out << setprecision(5)
      << setw(w) << ss.min
      << setw(w) << ss.bottom_quartile
      << setw(w) << ss.mean
      << setw(w) << ss.top_quartile
      << setw(w) << ss.max
    ;
  return out;
}


struct sample_desc {
  size_t min_sample_size;
  double variance;
  sample_desc(size_t mss, double v) : min_sample_size(mss), variance(v) { }
  ~sample_desc() { }
};


sample_desc get_sample(const vector<proc_data*>& procs, size_t region, size_t step, 
                       const cset *sample = NULL) {
  double sum = 0;
  double sum2 = 0;
  size_t N = (sample ? sample->size() : procs.size());   // sample size (for sample variance)

  if (!sample) {
    for (size_t p=0; p < procs.size(); p++) {
      double val = procs[p]->data(region, step);
      sum += val;
      sum2 += val * val;
    }
  } else {
    for (cset::iterator p = sample->begin(); p != sample->end(); p++) {
      double val = procs[*p]->data(region, step);
      sum += val;
      sum2 += val * val;
    }
  }

  double mean = sum/N;                               // sample mean
  double variance = (sum2/N - (mean*mean));       // estimate w/sample mean
  double stdDev = sqrt(variance);
  
  // calculate min sample size
  double Za = computeConfidenceInterval(confidence);   // double-tailed norm conf. interval
  double d = mean * error;                             // real error bound (error var is a %)
  double V = (d/(Za*stdDev));

  size_t min_sample_size = llround(N / (1 + N * V*V));

  return sample_desc(min_sample_size, variance);
}



int main(int argc, char **argv) {  
  get_args(argc, argv);
  
  if (argc != 1) usage();
  dirname = string(argv[0]);

  cout << "Directory:  " << dirname    << endl
       << "Level:      " << level      << endl
       << "Passes:     " << level      << endl
       << "Freq:       " << frequency  << endl
       << "Strata:     " << strata     << endl
       << "Conf:       " << confidence << endl
       << "Err:        " << error      << endl
       << "Iterations: " << iterations << endl;

  Timer timer;
  

  effort_dataset xdataset(dirname, -1, pass_limit);
  timer.record("Load exhaustive data");
  timer.dump();

  effort_dataset dataset(dirname, level, pass_limit);
  timer.record("Load data set");
  timer.dump();

  scale = dataset.procs() / dataset.rows();
  scale_levels = log2pow2(scale);
  cout << "Scale:         " << scale << endl;
  cout << "scale_levels:  " << scale_levels << endl;


  if (frequency != (frequency / scale)*scale) {
    cerr << "error: frequency must evenly divisible by scale." << endl;
    exit(1);
  }
    
  vector<proc_data*> xprocs;
  xdataset.transpose(xprocs);
  timer.record("Exhaustive Transpose");

  vector<proc_data*> procs;
  dataset.transpose(procs);
  timer.record("Transpose");
  timer.dump();


  // init kmedoids with only one cluster and everything in a straight line for simplicity
  kmedoids xkm(xprocs.size());
  kmedoids km(procs.size());

  cluster_list xclusters;
  xkm.to_cluster_list(xclusters);

  cluster_list clusters;
  km.to_cluster_list(clusters);

  timer.record("Initial cluster");
  timer.dump();

  // simulated time steps thru data.
  size_t num_good = 0;
  size_t updates = 0;
  size_t step;
  for (step=0; step < dataset.steps(); step++) {
    if (step && (step % frequency == 0)) {
      timer.clear();

      int xstart = step - frequency;
      if (xstart < 0) xstart = 0;
      int start = xstart / scale;

      cerr << "xstart: " << xstart << "     step: " << step<< endl;
      cerr << "start: " << start << "  ministep: " << step/scale<< endl;

      distance_fun xdist(xstart, step);
      xkm.clara(xprocs, xdist, strata);
      timer.record("Exhaustive CLARA Cluster");
      timer.dump();

      distance_fun dist(start, step/scale);
      km.clara(procs, dist, strata);
      timer.record("CLARA Cluster");
      timer.dump();

      cout << "tolisting" << endl;
      xkm.to_cluster_list(xclusters);
      km.to_cluster_list(clusters);  // need to expand this to xclusters level.
      cout << "done." << endl;

      cout << "expanding by " << scale_levels << endl;
      cout << "clusters sizes:" << endl;
      for (size_t i=0; i < clusters.size(); i++) cout << clusters[i].size() << " ";
      cout << endl;
      expand(clusters, scale_levels);
      for (size_t i=0; i < clusters.size(); i++) cout << clusters[i].size() << " ";
      cout << endl;
      cout << "done" << endl;

      timer.record("Convert + expand");
      timer.dump();

      // print out mirkin distance
      cout << "Mirkin distance: " << mirkin_distance(xclusters, clusters) << endl;

      updates++;
      if (updates == iterations) break;
    }

    vector<double> all_sizes;
    vector<double> all_variances;
    vector<double> sum_sizes;

    vector<double> cluster_sizes[strata];
    vector<double> cluster_variances[strata];


    for (size_t region=0; region < dataset.size(); region++) {
      size_t sample_size = 0;
      
      // iterate over each stratum
      for (size_t i=0; i < clusters.size(); i++) {
        //sample_desc sample = get_sample(procs, region, step, &clusters[i]);
        
        // expanded clusters points into xprocs.
        sample_desc sample = get_sample(xprocs, region, step, &clusters[i]);  

        cluster_sizes[i].push_back(sample.min_sample_size);
        cluster_variances[i].push_back(sample.variance);

        sample_size += sample.min_sample_size;
      }
      
      sample_desc all_sample = get_sample(xprocs, region, step);
      all_sizes.push_back(all_sample.min_sample_size);
      all_variances.push_back(all_sample.variance);
      sum_sizes.push_back(sample_size);
    }

    double all_size = summary(all_sizes).mean;
    double sum_size = summary(sum_sizes).mean;

    bool good = (sum_size < all_size);
    if (good) num_good++;

    cout << left  << setw(10) << "Iter " << step
         << right << setw(12) << all_size
         << right << setw(15) << sum_size
         << right << setw(15) << summary(all_variances).mean 
         << right << setw(15) << (good ? "GOOD" : "")
         << endl;

    for (size_t i=0; i < strata; i++) {
      if (cluster_sizes[i].size()) {
        cout << right << setw(10) << ""
             << left  << setw(12) << i
             << right << setw(15) << summary(cluster_sizes[i]).mean
             << right << setw(15) << summary(cluster_variances[i]).mean 
             << endl;
      }
    }
    cout << endl;
  }
  
  double percent = (double)num_good / step * 100;
  cout << num_good << "/" << step
       << " (" << setprecision(3) << percent << "%) good" << endl;
}

