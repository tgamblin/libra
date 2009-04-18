#ifndef SUMMARY_H
#define SUMMARY_H

#include "wavelet.h"
#include <iostream>

class Summary {
public:
  double mean()  { check_compute(); return mMean;  }
  double max()   { check_compute(); return mMax;   }
  double min()   { check_compute(); return mMin;   }
  double total() { check_compute(); return mTotal; }
  double count() { check_compute(); return mCount; }

  double meanRowDeviation() { check_compute(); return mMeanRowDeviation; }
  double maxRowDeviation()  { check_compute(); return mMaxRowDeviation;  }
  double minRowDeviation() { check_compute(); return mMinRowDeviation;  }

  double meanRowVariance() { check_compute(); return mMeanRowVariance; }
  double maxRowVariance()  { check_compute(); return mMaxRowVariance; }
  double minRowVariance()  { check_compute(); return mMinRowVariance; }

  double meanRowSkew() { check_compute(); return mMeanRowSkew; }
  double maxRowSkew()  { check_compute(); return mMaxRowSkew;  }
  double minRowSkew()  { check_compute(); return mMinRowSkew;  }
  
  double meanRowKurtosis() {check_compute(); return mMeanRowKurtosis; }
  double maxRowKurtosis()  {check_compute(); return mMaxRowKurtosis;  }
  double minRowKurtosis()  {check_compute(); return mMinRowKurtosis;  }

  Summary() : m(NULL), computed (false) { }
  ~Summary() { }

  void set_matrix(const wavelet::wt_matrix& new_matrix);
  void set_start_row(size_t r) { start_row = r; computed = false; }
  void set_end_row(size_t r)   { end_row = r;   computed = false; }
  void set_start_col(size_t c) { start_col = c; computed = false; }
  void set_end_col(size_t c)   { end_col = c;   computed = false; }

private:
  void check_compute() { 
    if (!m) {
      std::cerr << "Error: matrix not set!" << std::endl;
      exit(1);
    }

    if (!computed) {
      compute();
      computed = true;
    }
  }

  void compute();

  const wavelet::wt_matrix *m;
  bool computed;
  size_t start_row;
  size_t end_row;
  size_t start_col;
  size_t end_col;

  // below variables are all set by compute()
  double mMean;
  double mMax;
  double mMin;
  double mTotal;
  size_t mCount;
  
  double mMeanRowDeviation;
  double mMaxRowDeviation;
  double mMinRowDeviation;

  double mMeanRowVariance;
  double mMaxRowVariance;
  double mMinRowVariance;

  double mMeanRowSkew;
  double mMaxRowSkew;
  double mMinRowSkew;
  
  double mMeanRowKurtosis;
  double mMaxRowKurtosis;
  double mMinRowKurtosis;
};

#endif // SUMMARY_H
