#ifndef VTK_Z_VALUE_SCALARS_H
#define VTK_Z_VALUE_SCALARS_H

#include "vtkProgrammableFilter.h"

class vtkZValueScalars : public vtkProgrammableFilter {
public:
  static vtkZValueScalars *New();
  vtkTypeRevisionMacro(vtkZValueScalars, vtkProgrammableFilter);
  static void SetScalarsToInputPointZValues(void *filter);

protected:
  vtkZValueScalars();
  virtual ~vtkZValueScalars();

private:
  vtkZValueScalars(const vtkZValueScalars& other);            // not implemented
  vtkZValueScalars& operator=(const vtkZValueScalars& other); // not implemented
};


#endif // VTK_Z_VALUE_SCALARS_H
