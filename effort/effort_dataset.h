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
