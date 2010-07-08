/////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010, Lawrence Livermore National Security, LLC.  
// Produced at the Lawrence Livermore National Laboratory  
// Written by Todd Gamblin, tgamblin@llnl.gov.
// LLNL-CODE-417602
// All rights reserved.  
// 
// This file is part of Libra. For details, see http://github.com/tgamblin/libra.
// Please also read the LICENSE file for further information.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the disclaimer below.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the disclaimer (as noted below) in the documentation and/or other materials
//    provided with the distribution.
//  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
//    or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
/////////////////////////////////////////////////////////////////////////////////////////////////
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



void usage() {
  cerr << "Usage: approx-timer [options] dir" << endl;
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


void get_procs(vector<proc_data*>& procs, Timer& timer, string dirname, size_t pass_limit) {
  effort_dataset dataset(dirname, level, pass_limit);
  timer.record("Load data set");
  timer.dump();
  dataset.transpose(procs);
  timer.record("Transpose");
  timer.dump();
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
  

  vector<proc_data*> procs;
  get_procs(procs, timer, dirname, pass_limit);

  // init kmedoids with only one cluster and everything in a straight line for simplicity
  kmedoids km;
  timer.record("construct km");
  km.clara(procs, distance_fun(0, frequency), strata);
  timer.record("clustering");
  timer.dump();
}

