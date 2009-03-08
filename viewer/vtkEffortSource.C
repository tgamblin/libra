#include "vtkEffortSource.h"
#include "vtkObjectFactory.h"

#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
using namespace std;

#include "vtkEffortData.h"
#include "vtkPlaneSource.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkWarpScalar.h"
#include "vtkProgrammableFilter.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkDoubleArray.h"

#include "wt_direct.h"
#include "ezw_decoder.h"
#include "matrix_utils.h"
using namespace wavelet;

#include "wavelet_ssim.h"
using namespace effort;

vtkCxxRevisionMacro(vtkEffortSource, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkEffortSource);


vtkEffortSource::vtkEffortSource() 
  : vtkWarpScalar(), 
    scale_ratio(.5), 
    empty(true),
    is_wavelet(false), 
    normalize(true), 
    plane(vtkPlaneSource::New()),
    transform(vtkTransform::New()),
    transF(vtkTransformPolyDataFilter::New()),
    warpfil(vtkProgrammableFilter::New())
{ 
  // initially set bounds.
  x_min = 0;  x_max = 1;
  y_min = 0;  y_max = 1;
  z_min = 0;  z_max = 1;
  
  transF->SetInputConnection(plane->GetOutputPort());
  transF->SetTransform(transform);
  warpfil->SetInputConnection(transF->GetOutputPort());

  warpfil->SetExecuteMethod(WarpHandler, this);
  this->SetInput(warpfil->GetPolyDataOutput());
  this->ScaleFactor = 1;  // initially set to 1
}


vtkEffortSource::~vtkEffortSource() {
	plane->Delete();
  transform->Delete();
  transF->Delete();
  warpfil->Delete();
}


void vtkEffortSource::WarpHandler(void *obj) {
  vtkEffortSource *self = static_cast<vtkEffortSource*>(obj);
  self->DoEffortWarping();
}


void vtkEffortSource::DoEffortWarping() {
  vtkPolyData *input = warpfil->GetPolyDataInput();
  size_t numPts = input->GetNumberOfPoints();

  vtkPoints *newPts = vtkPoints::New();
  vtkDoubleArray *scalars = vtkDoubleArray::New();

  for (size_t i=0; i < numPts; i++) {
    double *point = input->GetPoint(i);
    size_t row = (size_t)point[0];
    size_t col = (size_t)point[1];

    double effort = 0;
    if (in_bounds(mat, row, col)) {
      effort = mat(row, col);
    } else {
      cerr << "ERROR: out of bounds ("  << row << ", " << col << ") " 
           << "from " << "(" << point[0] << ", " << point[1] << ")" << endl;
    }

    newPts->InsertPoint(i, row, col, 0);
    scalars->InsertValue(i, effort);
  }
  
  warpfil->GetPolyDataOutput()->CopyStructure(input);
  warpfil->GetPolyDataOutput()->SetPoints(newPts);
  warpfil->GetPolyDataOutput()->GetPointData()->SetScalars(scalars);

  // reference counting -- this just deletes here.
  newPts->Delete();
  scalars->Delete();
}


void vtkEffortSource::AddEffort(vtkEffortData *ereg) {
  const wt_matrix& emat = is_wavelet ? ereg->getCoefficients() : ereg->getData();

  // resize and clear things out on first add.
  if (empty) {
    empty = false;
    mat.resize(emat.size1(), emat.size2());
    mat.clear();

    // x and y bounds are the same normalized or not (they're just indices)
    x_min = 0;  x_max = mat.size1();
    y_min = 0;  y_max = mat.size2();
    UpdateXYBounds();
  }

  if (mat.size1() != emat.size1() || mat.size2() != emat.size2()) {
    cerr << "ERROR: effort sizes don't agree." << endl;
    exit(1);
  }

  mat += emat;
  UpdateZBounds();
}


// call this whenever x and y bounds change
// also need to call UpdateZBounds after in most cases.
void vtkEffortSource::UpdateXYBounds() {
  size_t rows = x_max - x_min;
  size_t cols = y_max - y_min;

  // Get plane to have the same resolution as ranks and 
  // timesteps in the matrix.
  plane->SetXResolution(rows - 1);
  plane->SetYResolution(cols - 1);
  plane->SetCenter(0.5, 0.5, 0);

  // Scale things up so that we get integer indices for the matrix.
  // note this is done only once when the first effort is added.
  transform->Delete();
  transform = vtkTransform::New();
  transform->Translate(x_min, y_min, 0);
  transform->Scale(rows - 1, cols - 1, 1);
  transF->SetTransform(transform);
}


void vtkEffortSource::UpdateZBounds() {
  // update boundaries after we change the data
  z_min = min_val(mat, x_min, x_max, y_min, y_max);
  z_max = max_val(mat, x_min, x_max, y_min, y_max);

  // widen the bounds a bit if things are too even.
  if ((z_min - z_max) < 1e-6) {
    z_max += 1;
  }
  
  UpdateScaleFactor();
}

void vtkEffortSource::UpdateScaleFactor() {
  // set scale factor and bounds so that data gets normalized properly
  ScaleFactor = normalize 
    ? (scale_ratio * min(mat.size1(), mat.size2())) / (z_max - z_min)
    : 1;
}


void vtkEffortSource::SetRowRange(int start, int end) {
  assert(start >= 0 && start <= (int)mat.size1());
  assert(end   >= 0 && end   <= (int)mat.size1());

  // bounds and normalized_bounds are the same on xy plane.
  x_min = start;
  x_max = end;

  UpdateXYBounds();
  UpdateZBounds();
}

void vtkEffortSource::SetColRange(int start, int end) {
  assert(start >= 0 && start <= (int)mat.size2());
  assert(end   >= 0 && end   <= (int)mat.size2());

  y_min = start;
  y_max = end;

  UpdateXYBounds();
  UpdateZBounds();
}


int *vtkEffortSource::GetRowRange() {
  row_range[0] = (int)x_min;
  row_range[1] = (int)x_max;
  return row_range;
}

int *vtkEffortSource::GetColRange() {
  col_range[0] = (int)y_min;
  col_range[1] = (int)y_max;
  return col_range;
}

double *vtkEffortSource::GetBounds() {
  bounds[0] = x_min;
  bounds[1] = x_max;
  bounds[2] = y_min;
  bounds[3] = y_max;
  bounds[4] = z_min;
  bounds[5] = z_max;
  return bounds;
}

double *vtkEffortSource::GetNormalizedBounds() {
  // normalized bounds need to be recalculated on fetch.
  normalized_bounds[0] = x_min;
  normalized_bounds[1] = x_max;
  normalized_bounds[2] = y_min;
  normalized_bounds[3] = y_max;
  normalized_bounds[4] = z_min * ScaleFactor;
  normalized_bounds[5] = z_max * ScaleFactor;

  return normalized_bounds;
}

void vtkEffortSource::SetScaleRatio(double ratio) {
  scale_ratio = ratio;
  UpdateScaleFactor();
}

double vtkEffortSource::GetScaleRatio() {
  return scale_ratio;
}

void vtkEffortSource::SetRenderWavelets(int render) {
  is_wavelet = render;
}

void vtkEffortSource::SetNormalize(int n) {
  normalize = n;
  UpdateScaleFactor();
}

int vtkEffortSource::Rows() {
  return mat.size1();
}
  

int vtkEffortSource::Cols() {
  return mat.size2();
}


double vtkEffortSource::GetValue(int row, int col) {
  return mat(row, col);
}


double vtkEffortSource::GetNormalizedValue(int row, int col) {
  return mat(row, col) * ScaleFactor;
}
