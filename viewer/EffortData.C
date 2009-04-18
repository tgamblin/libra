#include "EffortData.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <stdint.h>
using namespace std;

#include "wt_direct.h"
#include "ezw_decoder.h"
#include "matrix_utils.h"
using namespace wavelet;

#include "wavelet_ssim.h"
using namespace effort;

#include "vtkEffortData.h"

EffortData::EffortData(const string& fn) : filename(fn), approximation_level(-1), loaded(false) { 
  filename = string(filename);
  ifstream in(filename.c_str());
  if (in.fail()) {
    cerr << "Couldn't open file: " << filename << endl;
    exit(1);
  }

  // just read in the metadata here
  effort_key::read_in(in, id);
  ezw_header::read_in(in, header);
} 


EffortData::~EffortData() {  }


string EffortData::getVTKEffortData() {
  vtkEffortData *vtkRegion = vtkEffortData::New();
  vtkRegion->setEffortData(this);

  ostringstream mangled_pointer;
  mangled_pointer << "_" << hex << (uintptr_t)vtkRegion << "_p_vtkEffortData";
  return mangled_pointer.str();
}


void EffortData::load_from_file() const {
  ifstream in(filename.c_str());
  if (in.fail()) {
    cerr << "Couldn't open file: " << filename << endl;
    exit(1);
  }

  // have to read in the metadata again to get to the data, but we discard it here.
  effort_key key;
  ezw_header hdr;
  effort_key::read_in(in, key);
  ezw_header::read_in(in, hdr);

  // here we read in wavelet coefficients from the ezw stream.
  ezw_decoder decoder;
  decoder.set_pass_limit(pass_limit);
  int level = decoder.decode(in, wt, approximation_level, &hdr);

  // now inverse-transform
  wt_direct dwt;
  mat = wt;
  dwt.iwt_2d(mat, level);
  
  // scale up based on approximation level
  mat *= (1<<(header.level - approximation_level));
  
  summary.set_matrix(mat);
  loaded = true;
}

const wt_matrix& EffortData::getData() const {
  load();
  return mat;
}


const wt_matrix& EffortData::getCoefficients() const {
  load();
  return wt;
}

double EffortData::rmse(EffortData *other) {
  return ::rmse(getData(), other->getData());
}


double EffortData::wtrmse(EffortData *other) {
  return ::rmse(getCoefficients(), other->getCoefficients());
}

ezw_header EffortData::getHeader() {
  return header;
}

std::string EffortData::getFilename() {
  return filename;
}

Callpath EffortData::getStart() {
  return id.start_path;
}

Callpath EffortData::getEnd() {
  return id.end_path;
}

int EffortData::getType() {
  return id.type;
}

std::string EffortData::getMetric() {
  return id.metric.str();
}

void EffortData::setApproximationLevel(int level) {
  approximation_level = level;
}

void EffortData::setPassLimit(size_t limit) {
  pass_limit = limit;
}


