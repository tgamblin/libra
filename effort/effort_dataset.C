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
#include "effort_dataset.h"

#include <dirent.h>
#include <fstream>
using namespace std;

#include "ezw_decoder.h"
#include "wt_direct.h"
using namespace wavelet;

#include "effort_key.h"

namespace effort {


  // ---------------------------------------------------------------------- //
  // region
  // ---------------------------------------------------------------------- //
  region::region(const string& filename, int approximation_level, size_t pass_limit) {
    ifstream in(filename.c_str());
    if (in.fail()) {
      cerr << "Couldn't open file: " << filename << endl;
      exit(1);
    }
    
    // have to read in the metadata again to get to the data, but we discard it here.
    effort_key::read_in(in, key);
    ezw_header::read_in(in, header);
    
    // here we read in wavelet coefficients from the ezw stream.
    ezw_decoder decoder;
    decoder.set_pass_limit(pass_limit);
    int level = decoder.decode(in, mat, approximation_level, &header);
    
    // now inverse-transform
    wt_direct dwt;
    dwt.iwt_2d(mat, level);
  }

  region::~region() { }

  
  // ---------------------------------------------------------------------- //
  // proc_data
  // ---------------------------------------------------------------------- //
  proc_data::proc_data(size_t rows, size_t cols) : data(rows, cols) { }
  proc_data::~proc_data() { }


  // ---------------------------------------------------------------------- //
  // effort_dataset
  // ---------------------------------------------------------------------- //
  effort_dataset::effort_dataset(const string& dir, int level, size_t pass_limit) 
    : directory(dir), approximation_level(level) 
  { 
    DIR *dirp = opendir(directory.c_str());
    if (!dirp) {
      cerr << "Error opening directory: '" << directory << "'" << endl;
      exit(1);
    }
    
    effort_key key;
    bool first = true;
    for (dirent *dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
      if (parse_filename(dp->d_name)) {
        ostringstream sfullpath;
        sfullpath << directory << "/" << dp->d_name;
        string fullpath = sfullpath.str();
        
        region *r = new region(fullpath, level, pass_limit);
        regions[r->key] = r;
        
        if (first) {
          header = r->header;

          if (approximation_level < 0) {
            approximation_level = header.level;
          }

          if ((size_t)approximation_level > r->header.level) {
            cerr << "Can't expand to approx level " << level 
                 << " when actual level is " << header.level << endl;
            exit(1);
          }
          mRows = r->mat.size1();
          mCols = r->mat.size2();
          first = false;
        }
      }
    }
  }
  
  effort_dataset::effort_dataset(const effort_dataset& other) 
    :directory(other.directory), approximation_level(other.approximation_level),
     regions(other.regions), header(other.header), mRows(other.mRows), mCols(other.mCols)
  { }


  effort_dataset::~effort_dataset() { }


  effort_dataset& effort_dataset::operator=(const effort_dataset& other) {
    directory = other.directory;
    approximation_level = other.approximation_level;
    regions = other.regions;
    header = other.header;
    mRows = other.mRows;
    mCols = other.mCols;
    return *this;
  }

  
  void effort_dataset::standardize() {
    for (map<effort_key, region*>::iterator i=regions.begin(); i != regions.end(); i++) {
      ::standardize(i->second->mat);
    }
  }


  void effort_dataset::transpose(vector<proc_data*>& trans) {
    trans.clear();
    trans.resize(mRows);  // resize to approx num procs

    for (size_t i=0; i < mRows; i++) {
      trans[i] = new proc_data(size(), mCols); // regions  x timesteps
    }

    size_t r=0;
    for (map<effort_key, region*>::iterator i=regions.begin(); i != regions.end(); i++) {
      region *region = i->second;
      for (size_t i=0; i < mRows; i++) {
        for (size_t j=0; j < mCols; j++) {
          // transpose has a row per region and a mat per process
          trans[i]->data(r,j) = region->mat(i,j);
        }
      }
      r++;
    }    
  }
  
} // namespace effort
