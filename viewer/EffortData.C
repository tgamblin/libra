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

#include "vtkPythonUtil.h"
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
  return string(vtkPythonManglePointer(vtkRegion, "p_vtkEffortData"));
}


void EffortData::load_from_file() {
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

  // if the caller set the approximation level too high (e.g. we don't have enough 
  // data for that resolution), then adjust things here.
  if (approximation_level > (int)hdr.level) {
    approximation_level = hdr.level;
  }

  // here we read in wavelet coefficients from the ezw stream.
  ezw_decoder decoder;
  decoder.set_pass_limit(pass_limit);
  int level = decoder.decode(in, wt, approximation_level, &hdr);

  // now inverse-transform
  wt_direct dwt;
  mat = wt;
  dwt.iwt_2d(mat, level);
  
  // scale up based on approximation level
  int scale = (1<<(header.level - approximation_level));
  mat *= scale;
  
  summary.set_matrix(mat);
  loaded = true;
}

const wt_matrix& EffortData::getData() {
  load();
  return mat;
}


const wt_matrix& EffortData::getCoefficients() {
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


