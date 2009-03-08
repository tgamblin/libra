// -*- c++ -*-
%module pyeffort

%{
#include <sstream>
#include <stdint.h>
#include "Callpath.h"
#include "EffortData.h"
#include "FrameId.h"
#include "effort_key.h"
#include "ezw.h"
using namespace wavelet;
using namespace effort;
using namespace std;

#include <Python.h>
#include <vtkPythonUtil.h>
%}

%include "stl.i"
%include "std_string.i"
%include "typemaps.i"
%pythoncode %{
  import re
%}

// --------------------------------------------------- //
// Callpath
// --------------------------------------------------- //
%rename(__getitem__) get;   // allow indexing
%rename(__len__) size;      // be like python containers
class Callpath {
public:
  Callpath() : path(NULL);
  Callpath(const Callpath& other);
  ~Callpath();

  static inline Callpath null();
  const FrameId& get(size_t i) const;
  size_t size() const;
  void write_out(std::ostream& out);
  static Callpath read_in(std::istream& in);

%pythoncode %{
    # Support iteration over frames in a callpath
    def __iter__(self):
      for i in xrange(0,len(self)):
        yield self[i]

    # Easy, slow hash function.  Make it faster later if needed.
    def __hash__(self):
      return hash(str(self))
%}
}; // Callpath


%extend Callpath {
  std::string __str__() {
    ostringstream outs;
    outs << *self;
    return outs.str();
  }

  bool __eq__(const Callpath *other) {
    if (self && other) {
      return (*self) == (*other);
    } else {
      return self == other;
    }
  }
  bool __lt__(const Callpath *other) {
    if (self && other) {
      return (*self) < (*other);
    } else {
      return self < other;
    }
  }
  bool __gt__(const Callpath *other) {
    if (self && other) {
      return (*self) > (*other);
    } else {
      return self > other;
    }
  }
}


// --------------------------------------------------- //
// ezw_header
// --------------------------------------------------- //
%apply long long {quantized_t mean};
%apply long long {quantized_t threshold};
%apply int {encoding_t enc_type};
struct ezw_header {
  size_t rows;
  size_t cols;
  size_t level;
  quantized_t mean;
  unsigned long long scale;
  quantized_t threshold;
  encoding_t enc_type;
  size_t blocks;
  size_t passes;
  size_t ezw_size;
  size_t rle_size;
  size_t enc_size;

  ezw_header();
  ezw_header(size_t r, size_t c, int l, quantized_t m, unsigned long long s, quantized_t t,
             encoding_t et = ARITHMETIC, size_t b = 1, size_t p = 0);
  size_t write_out(std::ostream& out);
  static void read_in(std::istream& in, ezw_header& header);
};

%extend ezw_header { 
  std::string __str__() {
    ostringstream outs;
    outs << *self;
    return outs.str();
  }
}

// --------------------------------------------------- //
// EffortData
// --------------------------------------------------- //
// Need to import vtk to get at vtk.vtkObject.
%pythoncode %{
import vtk
%}

// This makes getVTKEffortData() work like you'd expect it to.
%feature("shadow") EffortData::getVTKEffortData() %{
def getVTKEffortData(*args):
    return vtk.vtkObject(str($action(*args)))
%}

class EffortData {
public:
  ezw_header getHeader();
  std::string getFilename();
  Callpath getStart();
  Callpath getEnd();
  int getType();
  std::string getMetric();
  void setApproximationLevel(int level);
  void load() const;

  size_t rows();
  size_t cols();
  double getValue(size_t row, size_t col);

  EffortData(const std::string& filename);
  ~EffortData();

  double rmse(EffortData *other);
  double wtrmse(EffortData *other);
  std::string getVTKEffortData();

%pythoncode %{
  def __getitem__(self, key):
    r,c = key
    return self.getValue(r,c)
%}

private:
  EffortData(const EffortData& other);
  EffortData& operator=(const EffortData& other);
};


%pythoncode %{
import source
%}

// --------------------------------------------------- //
// FrameId
// --------------------------------------------------- //
class FrameId {
public:
  ~FrameId() { }
  FrameId(const FrameId& other);
  void write_out(std::ostream& out) const;
  static FrameId read_in(std::istream& in);
  static FrameId create(const std::string& name, uintptr_t offset);

  friend class Callpath;

%pythoncode %{
  # Easy, slow hash function.  Make it faster later if needed.
  def __hash__(self):
    return hash(str(self))

  def file(self):
    sym = source.getSymbol((self.module(), self.offset()))
    if sym: return sym.file
    return None

  def line(self):
    sym = source.getSymbol((self.module(), self.offset()))
    if sym: return sym.line
    return None

  def fun(self):
    sym = source.getSymbol((self.module(), self.offset()))
    if sym: return sym.fun
    return None
%}
};

%extend FrameId {
  std::string __str__() {
    ostringstream outs;
    outs << *self;
    return outs.str();
  }

  bool __eq__(const FrameId *other) {
    if (self && other) {
      return (*self) == (*other);
    } else {
      return self == other;
    }
  }
  bool __lt__(const FrameId *other) {
    if (self && other) {
      return (*self) < (*other);
    } else {
      return self < other;
    }
  }
  bool __gt__(const FrameId *other) {
    if (self && other) {
      return (*self) > (*other);
    } else {
      return self > other;
    }
  }

  const std::string& module() {
    return *self->module;
  }

  unsigned long long offset() {
    return self->offset;
  }
}


