#ifndef EFFORT_DATA_SET_H
#define EFFORT_DATA_SET_H

#include <string>
#include <map>
#include <vector>
#include <string>

#include "wavelet.h"
#include "ezw.h"
#include "effort_key.h"

namespace effort {

  struct region {
    region(const std::string& filename, int approximation_level=-1, size_t pass_limit=0);
    ~region();
    
    effort_key key;
    wavelet::ezw_header header;
    wavelet::wt_matrix mat;
  }; // region



  struct proc_data {
    wavelet::wt_matrix data;
    proc_data(size_t rows, size_t cols);
    ~proc_data();
  };


  class effort_dataset {
  public:
    effort_dataset(const std::string& directory, 
                   int approximation_level=-1, size_t pass_limit=0);

    effort_dataset(const effort_dataset& other);
    ~effort_dataset();
    
    // subtracts out mean and divides by stddev for all data in this dataset 
    void standardize();

    // splits matrices up into per-process matrices for clustering.
    void transpose(std::vector<proc_data*>& trans);

    size_t rows() { return mRows; }
    size_t cols() { return mCols; }
    size_t size() { return regions.size(); }

    size_t procs() { return mRows >> approximation_level << header.level; }
    size_t steps() { return mCols >> approximation_level << header.level; }
    
    effort_dataset& operator=(const effort_dataset& other);
  private:
    std::string directory;
    int approximation_level;
    std::map<effort_key, region*> regions;
    wavelet::ezw_header header;
    size_t mRows;
    size_t mCols;
  }; // effort_dataset
  
}

#endif // EFFORT_DATA_SET_H
