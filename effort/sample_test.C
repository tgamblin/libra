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

#include "Dissimilarity.h"
#include "KMedoids.h"
#include "ltqnorm.h"

string dirname;
int level;
size_t freq;
double confidence = 0;
double error = 0;


void usage() {
  cerr << "Usage: sample_replay <dir> <level> <freq> <conf> <err>" << endl;
  cerr << "Args: " << endl;
  cerr << "  dir     A directory full of effort files." << endl;
  cerr << "  level   Approximation level for data analysis." << endl;
  cerr << "  freq    Sample frequency (in timesteps)" << endl;
  cerr << "  conf    Confidence bound." << endl;
  cerr << "  err     Error bound (percent)" << endl;
}



struct nrmse_distance : public Dissimilarity<proc_data*> {
    virtual double getDissimilarity(proc_data* left, proc_data* right) const {
        return nrmse(left->data, right->data);
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
    min = numbers[0];
    max = numbers[numbers.size() - 1];
    double quartile_size = numbers.size() / 4.0;
    top_quartile = numbers[(size_t)floor(numbers.size() - quartile_size)];
    bottom_quartile = numbers[(size_t)floor(quartile_size)];

    mean = sum / numbers.size();
  }
};

ostream& operator<<(ostream& out, const summary& ss) {
  const int w=10;
  out << setw(w) << ss.min
      << setw(w) << ss.bottom_quartile
      << setw(w) << ss.mean
      << setw(w) << ss.top_quartile
      << setw(w) << ss.max
    ;
  return out;
}




size_t get_sample_size(vector<proc_data*>& procs, size_t region, size_t step, KMedoids::cluster *sample = NULL) {
  double sum = 0;
  double sum2 = 0;

  size_t size = sample ? sample->size() : procs.size();

  if (!sample) {
    for (size_t p=0; p < procs.size(); p++) {
      double value = procs[p]->data(region, step);
      sum  += value;
      sum2 += value * value;
    }
  } else {
    for (KMedoids::cluster::iterator p = sample->begin(); p != sample->end(); p++) {
      double value = procs[*p]->data(region, step);
      sum  += value;
      sum2 += value * value;
    }
  }
  
  double mean = sum / size;
  double variance = (sum2 - (sum * sum)/size) / size;

  // calculate min sample size
  double Za = computeConfidenceInterval(confidence);   // double-tailed norm conf. interval
  double d = mean * error;                              // real error bound (error var is a %)
  double stdDev = sqrt(variance);
      
  double V = (d/(Za*stdDev));
  size_t min_sample_size = llround(size * 1/(1 + size * V*V));

  return min_sample_size;
}



int main(int argc, char **argv) {
  if (argc != 6) usage();
  char *err;

  dirname = string(argv[1]);

  level = strtol(argv[2], &err, 0);
  if (*err) usage();

  freq = strtol(argv[3], &err, 0);
  if (*err) usage();

  confidence = strtod(argv[4], &err);
  if (*err) usage();

  error = strtod(argv[5], &err);
  if (*err) usage();

  cout << "Directory: " << dirname
       << "Level: "    << level 
       << "    Freq: " << freq
       << "    Conf: " << confidence
       << "    Err: " << error
       << endl;
  
  effort_dataset dataset(dirname, level);

  vector<proc_data*> procs;
  dataset.transpose(procs);
  ClusterDataSet *cluster_data = ClusterDataSet::buildFromObjects(procs, nrmse_distance());
  
  
  // simulated time steps thru data.
  for (size_t step=0; step < dataset.num_steps(); step++) {
    vector<double> sizes;
    for (size_t region=0; region < dataset.num_regions(); region++) {
      sizes.push_back(get_sample_size(procs, region, step));
    }
    cout << left << setw(10) << step << summary(sizes) << endl;
  }
  
}

