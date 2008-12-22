#include "vtkZValueScalars.h"
#include "vtkObjectFactory.h"
#include "vtkFloatArray.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"


vtkCxxRevisionMacro(vtkZValueScalars, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkZValueScalars);


vtkZValueScalars::vtkZValueScalars() : vtkProgrammableFilter() { 
  this->SetExecuteMethod(&SetScalarsToInputPointZValues, this);
}

vtkZValueScalars::~vtkZValueScalars() { }

void vtkZValueScalars::SetScalarsToInputPointZValues(void *thisFilter) {
  vtkProgrammableFilter *filter = (vtkProgrammableFilter*)thisFilter;

  vtkPolyData *input = filter->GetPolyDataInput();
  int npts = input->GetNumberOfPoints();

  vtkFloatArray *scalars = vtkFloatArray::New();
  scalars->SetNumberOfValues(npts);
  
  double point[3];
  for (int i=0; i < npts; i++) {
    input->GetPoint(i, point);
    scalars->SetValue(i, point[2]);
  }
  
  filter->GetPolyDataOutput()->GetPointData()->SetScalars(scalars);
}
