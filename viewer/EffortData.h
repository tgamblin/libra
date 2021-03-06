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
#ifndef EFFORT_DATA_H
#define EFFORT_DATA_H

#include <string>
#include "wavelet.h"
#include "ezw.h"
#include "effort_key.h"
#include "Callpath.h"
#include "summary.h"

/// This class represents data and metadata from a particular effort
/// region monitored at runtme.  Each effort region 
class EffortData {
public:

  /// This reads in metadata but saves decompressesion for later.
  /// Filename is kept around so we can open it again later.
  EffortData(const std::string& filename);

  /// Destructor doesn't need to do anything.
  ~EffortData();

  // ------------------------------------------------------ //
  // Below methods are exposed to python.
  // ------------------------------------------------------ //
  wavelet::ezw_header getHeader();
  std::string getFilename();
  Callpath getStart();
  Callpath getEnd();
  int getType();
  std::string getMetric();


  /// Approximation level determines how many levels of the wavelet
  /// transform we'll expand.  Negative values indicate that we should
  /// expand all levels possible.  Small positive values result
  /// in a small internal approximation.  Larger values result in
  /// a larger, but more precise approximation of the original data.
  void setApproximationLevel(int level);

  void setPassLimit(size_t limit);

  /// Calculates rms error between two load functions
  double rmse(EffortData *other);

  /// Calculates rms over the wavelet coefficients
  double wtrmse(EffortData *other);

  size_t rows() { 
    return (approximation_level < 0) 
      ? header.rows 
      : (header.rows >> header.level << approximation_level);
  }

  size_t cols() { 
    return (approximation_level < 0) 
      ? header.cols
      : (header.cols >> header.level << approximation_level);
  }
  
  size_t processes() {
    return header.rows;
  }
  
  size_t steps() {
    return header.cols;
  }
  
  double getValue(size_t row, size_t col) {
    return mat(row, col);
  }

  /// Forces data to load from file.
  void load() {
    if (!loaded) {
      load_from_file(); // sets loaded to true.
    }
  }

  /// This returns a raw SWIG-style mangled pointer to a VTK object.  Can be used 
  /// by the vtk python wrappers to get at EffortDatas.  The vtkEffortData
  /// returned indirectly references the instance of EffortData it came from.
  std::string getVTKEffortData();

  // ------------------------------------------------------ //
  // Below methods are unexposed to python -- just for
  // other C++ classes to use.
  // ------------------------------------------------------ //
  /// Get data matrix.  This will lazily load data from the file.
  const wavelet::wt_matrix& getData();
  
  /// Get coefficients matrix.  Lazily loads data from file.
  const wavelet::wt_matrix& getCoefficients();

  // summary statistics
  double mean()             { load(); return summary.mean();  }
  double max()              { load(); return summary.max();   }
  double min()              { load(); return summary.min();   }
  double total()            { load(); return summary.total(); }
  double count()            { load(); return summary.count(); }

  double meanRowDeviation() { load(); return summary.meanRowDeviation(); }
  double maxRowDeviation()  { load(); return summary.maxRowDeviation();  }
  double minRowDeviation()  { load(); return summary.minRowDeviation();  }

  double meanRowVariance()  { load(); return summary.meanRowVariance(); }
  double maxRowVariance()   { load(); return summary.maxRowVariance(); }
  double minRowVariance()   { load(); return summary.minRowVariance(); }

  double meanRowSkew()      { load(); return summary.meanRowSkew(); }
  double maxRowSkew()       { load(); return summary.maxRowSkew();  }
  double minRowSkew()       { load(); return summary.minRowSkew();  }
  
  double meanRowKurtosis()  { load(); return summary.meanRowKurtosis(); }
  double maxRowKurtosis()   { load(); return summary.maxRowKurtosis();  }
  double minRowKurtosis()   { load(); return summary.minRowKurtosis();  }


private:
  wavelet::ezw_header header;           /// Header data
  effort::effort_key id;                /// Metadata
  std::string filename;                 /// Name of file that data came from.
  int approximation_level;              /// Level of approx to expand to.
  size_t pass_limit;                    /// Max # EZW passes to expand
  
  EffortData(const EffortData& other);            // not implemented
  EffortData& operator=(const EffortData& other); // not implemented

  void load_from_file();              /// Reads in matrix from file and does decompression

  wavelet::wt_matrix mat;     /// Spatial data (lazily loaded)
  wavelet::wt_matrix wt;      /// Wavelet coefficients from file (lazily loaded).
  bool loaded;                /// Whether matrices have been read in yet.
  Summary summary;
};


#endif // EFFORT_DATA_H
