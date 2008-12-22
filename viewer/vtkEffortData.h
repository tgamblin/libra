#ifndef VTK_EFFORT_DATA_H
#define VTK_EFFORT_DATA_H

#include "vtkObject.h"
#include "EffortData.h"
#include "wavelet.h"
using wavelet::wt_matrix;

/// This class represents data and metadata from a particular effort
/// region monitored at runtme.  Each effort region 
class vtkEffortData : public vtkObject {
public:
  static vtkEffortData *New();
  vtkTypeRevisionMacro(vtkEffortData, vtkObject);

  /// Sets the effort region this will delegate to.
  virtual void setEffortData(const EffortData *del) {
    delegate = del;
  }

  /// Get data matrix.  This will lazily load data from the file.
  const wt_matrix& getData() {
    return delegate->getData();
  }
  
  /// Get coefficients matrix.  Lazily loads data from file.
  const wt_matrix& getCoefficients() {
    return delegate->getCoefficients();
  }
  
protected:
  vtkEffortData();
  virtual ~vtkEffortData();

private:
  vtkEffortData(const vtkEffortData& other);            // not implemented
  vtkEffortData& operator=(const vtkEffortData& other); // not implemented

  const EffortData *delegate;
};


#endif // VTK_EFFORT_DATA_H
