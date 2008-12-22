#ifndef EFFORT_DATA_H
#define EFFORT_DATA_H

#include <string>
#include "wavelet.h"
#include "ezw.h"
#include "effort_metadata.h"
#include "Callpath.h"

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

  /// Calculates rms error between two load functions
  double rmse(EffortData *other);

  /// Calculates rms over the wavelet coefficients
  double wtrmse(EffortData *other);

  size_t rows() { 
    return mat.size1(); 
  }

  size_t cols() { 
    return mat.size2(); 
  }

  double getValue(size_t row, size_t col) {
    return mat(row, col);
  }

  /// Forces data to load from file.
  void load() const {
    if (!loaded) load_from_file();
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
  const wavelet::wt_matrix& getData() const;
  
  /// Get coefficients matrix.  Lazily loads data from file.
  const wavelet::wt_matrix& getCoefficients() const;

private:
  wavelet::ezw_header header;          /// Header data
  effort::effort_metadata metadata;    /// Metadata
  std::string filename;                /// Name of file that data came from.
  int approximation_level;              /// Level of approx to expand to.

  EffortData(const EffortData& other);            // not implemented
  EffortData& operator=(const EffortData& other); // not implemented

  void load_from_file() const;        /// Reads in matrix from file and does decompression

  bool loaded;                        /// Whether matrices have been read in yet.
  mutable wavelet::wt_matrix mat;     /// Spatial data (lazily loaded)
  mutable wavelet::wt_matrix wt;      /// Wavelet coefficients from file (lazily loaded).
};


#endif // EFFORT_DATA_H
