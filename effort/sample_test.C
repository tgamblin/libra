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

string dirname;           // effort directory
int level;                // approximation level for data set
size_t pass_limit;        // pass limit for data set
size_t frequency;         // sample scheme update interval
size_t strata;            // # clusters
double confidence = 0;    // confidence % for sampling  (bt/w 0 and 1)
double error = 0;         // error bound for sampling (bt/w 0 and 1... probably closer to 0)

double scale;             // scale factor calculated from approximation level.


void usage() {
  cerr << "Usage: sample-test <dir> <level> <pass_limit> <freq> <strata> <conf> <err>" << endl;
  cerr << "Args: " << endl;
  cerr << "  dir         A directory full of effort files." << endl;
  cerr << "  level       Approximation level for data analysis." << endl;
  cerr << "  pass_limit  Number of EZW passes to decompress" << endl;
  cerr << "  freq        Update frequency (in approx timesteps)" << endl;
  cerr << "  strata      Number of strata (per update)." << endl;
  cerr << "  conf        Confidence bound." << endl;
  cerr << "  err         Error bound (percent)" << endl;
  exit(1);
}


void get_args(int& argc, char**& argv) {
  if (argc != 8) usage();
  char *err;
  size_t arg = 1;

  dirname = string(argv[arg++]);

  level = strtol(argv[arg++], &err, 0);
  if (*err) usage();

  pass_limit = strtol(argv[arg++], &err, 0);
  if (*err) usage();

  frequency = strtol(argv[arg++], &err, 0);
  if (*err) usage();

  strata = strtol(argv[arg++], &err, 0);
  if (*err) usage();

  confidence = strtod(argv[arg++], &err);
  if (*err) usage();

  error = strtod(argv[arg++], &err);
  if (*err) usage();

  cout << "Directory: "  << dirname    << endl
       << "Level: "  << level      << endl
       << "Passes: " << level      << endl
       << "Freq: "   << frequency  << endl
       << "Strata: " << strata     << endl
       << "Conf: "   << confidence << endl
       << "Err: "    << error      << endl;
}

struct distance_fun {
  size_t start_col, end_col;

  distance_fun(size_t start, size_t end) : start_col(start), end_col(end) { }
  
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
  size_t n = (sample ? sample->size() : procs.size());   // sample size (for sample variance)

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

  double mean = sum / n;                               // sample mean
  double variance = (sum2/n - (mean*mean));       // estimate w/sample mean
  double stdDev = sqrt(variance);
  
  // calculate min sample size
  double Za = computeConfidenceInterval(confidence);   // double-tailed norm conf. interval
  double d = mean * error;                             // real error bound (error var is a %)
  double V = (d/(Za*stdDev));

  size_t N = (size_t)floor(n * scale);                 // population size
  size_t min_sample_size = llround(N / (1 + N * V*V));

  return sample_desc(min_sample_size, variance);
}



int main(int argc, char **argv) {  
  get_args(argc, argv);

  effort_dataset dataset(dirname, level, pass_limit);
  scale = dataset.procs() / dataset.rows();

  vector<proc_data*> procs;

  //effort_dataset standardized_data(dataset);
  //standardized_data.standardize();
  //standardized_data.transpose(procs);

  dataset.transpose(procs);

  // init kmedoids with only one cluster and everything in a straight line for simplicity
  kmedoids km(procs.size());
  cluster_list clusters;
  km.to_cluster_list(clusters);
  
  // simulated time steps thru data.
  size_t num_good = 0;
  for (size_t step=0; step < dataset.cols(); step++) {
    if (step % frequency == frequency-1) {
      dissimilarity_matrix mat;
      build_dissimilarity_matrix(procs, 
                                 distance_fun(step - frequency, step), 
                                 mat);
      km.pam(mat, strata);
      km.to_cluster_list(clusters);
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
        sample_desc sample = get_sample(procs, region, step, &clusters[i]);
        cluster_sizes[i].push_back(sample.min_sample_size);
        cluster_variances[i].push_back(sample.variance);

        sample_size += sample.min_sample_size;
      }
      
      sample_desc all_sample = get_sample(procs, region, step);
      all_sizes.push_back(all_sample.min_sample_size);
      all_variances.push_back(all_sample.variance);
      sum_sizes.push_back(sample_size);
    }

    double all_size = summary(all_sizes).mean;
    double sum_size = summary(sum_sizes).mean;

    bool good = (sum_size < all_size);
    if (good) num_good++;

    cout << left  << setw(10) << step
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
  
  double percent = (double)num_good / dataset.cols() * 100;
  cout << num_good << "/" << dataset.cols()
       << " (" << setprecision(3) << percent << "%) good" << endl;
}

