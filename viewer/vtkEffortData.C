#include "vtkEffortData.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkEffortData, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkEffortData);

vtkEffortData::vtkEffortData() : vtkObject(), delegate(NULL) { } 

vtkEffortData::~vtkEffortData() { }


