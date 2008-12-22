# This file was created automatically by SWIG 1.3.29.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _pyeffort
import new
new_instancemethod = new.instancemethod
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'PySwigObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


class PySwigIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, PySwigIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, PySwigIterator, name)
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    __swig_destroy__ = _pyeffort.delete_PySwigIterator
    __del__ = lambda self : None;
    def value(*args): return _pyeffort.PySwigIterator_value(*args)
    def incr(*args): return _pyeffort.PySwigIterator_incr(*args)
    def decr(*args): return _pyeffort.PySwigIterator_decr(*args)
    def distance(*args): return _pyeffort.PySwigIterator_distance(*args)
    def equal(*args): return _pyeffort.PySwigIterator_equal(*args)
    def copy(*args): return _pyeffort.PySwigIterator_copy(*args)
    def next(*args): return _pyeffort.PySwigIterator_next(*args)
    def previous(*args): return _pyeffort.PySwigIterator_previous(*args)
    def advance(*args): return _pyeffort.PySwigIterator_advance(*args)
    def __eq__(*args): return _pyeffort.PySwigIterator___eq__(*args)
    def __ne__(*args): return _pyeffort.PySwigIterator___ne__(*args)
    def __iadd__(*args): return _pyeffort.PySwigIterator___iadd__(*args)
    def __isub__(*args): return _pyeffort.PySwigIterator___isub__(*args)
    def __add__(*args): return _pyeffort.PySwigIterator___add__(*args)
    def __sub__(*args): return _pyeffort.PySwigIterator___sub__(*args)
    def __iter__(self): return self
PySwigIterator_swigregister = _pyeffort.PySwigIterator_swigregister
PySwigIterator_swigregister(PySwigIterator)

import re

class Callpath(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Callpath, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Callpath, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _pyeffort.new_Callpath(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _pyeffort.delete_Callpath
    __del__ = lambda self : None;
    __swig_getmethods__["null"] = lambda x: _pyeffort.Callpath_null
    if _newclass:null = staticmethod(_pyeffort.Callpath_null)
    def __getitem__(*args): return _pyeffort.Callpath___getitem__(*args)
    def __len__(*args): return _pyeffort.Callpath___len__(*args)
    def write_out(*args): return _pyeffort.Callpath_write_out(*args)
    __swig_getmethods__["read_in"] = lambda x: _pyeffort.Callpath_read_in
    if _newclass:read_in = staticmethod(_pyeffort.Callpath_read_in)
    # Support iteration over frames in a callpath
    def __iter__(self):
      for i in xrange(0,len(self)):
        yield self[i]

    # Easy, slow hash function.  Make it faster later if needed.
    def __hash__(self):
      return hash(str(self))

    def __str__(*args): return _pyeffort.Callpath___str__(*args)
    def __eq__(*args): return _pyeffort.Callpath___eq__(*args)
    def __lt__(*args): return _pyeffort.Callpath___lt__(*args)
    def __gt__(*args): return _pyeffort.Callpath___gt__(*args)
Callpath_swigregister = _pyeffort.Callpath_swigregister
Callpath_swigregister(Callpath)
Callpath_null = _pyeffort.Callpath_null
Callpath_read_in = _pyeffort.Callpath_read_in

class ezw_header(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, ezw_header, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, ezw_header, name)
    __repr__ = _swig_repr
    __swig_setmethods__["rows"] = _pyeffort.ezw_header_rows_set
    __swig_getmethods__["rows"] = _pyeffort.ezw_header_rows_get
    if _newclass:rows = property(_pyeffort.ezw_header_rows_get, _pyeffort.ezw_header_rows_set)
    __swig_setmethods__["cols"] = _pyeffort.ezw_header_cols_set
    __swig_getmethods__["cols"] = _pyeffort.ezw_header_cols_get
    if _newclass:cols = property(_pyeffort.ezw_header_cols_get, _pyeffort.ezw_header_cols_set)
    __swig_setmethods__["level"] = _pyeffort.ezw_header_level_set
    __swig_getmethods__["level"] = _pyeffort.ezw_header_level_get
    if _newclass:level = property(_pyeffort.ezw_header_level_get, _pyeffort.ezw_header_level_set)
    __swig_setmethods__["mean"] = _pyeffort.ezw_header_mean_set
    __swig_getmethods__["mean"] = _pyeffort.ezw_header_mean_get
    if _newclass:mean = property(_pyeffort.ezw_header_mean_get, _pyeffort.ezw_header_mean_set)
    __swig_setmethods__["scale"] = _pyeffort.ezw_header_scale_set
    __swig_getmethods__["scale"] = _pyeffort.ezw_header_scale_get
    if _newclass:scale = property(_pyeffort.ezw_header_scale_get, _pyeffort.ezw_header_scale_set)
    __swig_setmethods__["threshold"] = _pyeffort.ezw_header_threshold_set
    __swig_getmethods__["threshold"] = _pyeffort.ezw_header_threshold_get
    if _newclass:threshold = property(_pyeffort.ezw_header_threshold_get, _pyeffort.ezw_header_threshold_set)
    __swig_setmethods__["enc_type"] = _pyeffort.ezw_header_enc_type_set
    __swig_getmethods__["enc_type"] = _pyeffort.ezw_header_enc_type_get
    if _newclass:enc_type = property(_pyeffort.ezw_header_enc_type_get, _pyeffort.ezw_header_enc_type_set)
    __swig_setmethods__["blocks"] = _pyeffort.ezw_header_blocks_set
    __swig_getmethods__["blocks"] = _pyeffort.ezw_header_blocks_get
    if _newclass:blocks = property(_pyeffort.ezw_header_blocks_get, _pyeffort.ezw_header_blocks_set)
    __swig_setmethods__["passes"] = _pyeffort.ezw_header_passes_set
    __swig_getmethods__["passes"] = _pyeffort.ezw_header_passes_get
    if _newclass:passes = property(_pyeffort.ezw_header_passes_get, _pyeffort.ezw_header_passes_set)
    __swig_setmethods__["ezw_size"] = _pyeffort.ezw_header_ezw_size_set
    __swig_getmethods__["ezw_size"] = _pyeffort.ezw_header_ezw_size_get
    if _newclass:ezw_size = property(_pyeffort.ezw_header_ezw_size_get, _pyeffort.ezw_header_ezw_size_set)
    __swig_setmethods__["rle_size"] = _pyeffort.ezw_header_rle_size_set
    __swig_getmethods__["rle_size"] = _pyeffort.ezw_header_rle_size_get
    if _newclass:rle_size = property(_pyeffort.ezw_header_rle_size_get, _pyeffort.ezw_header_rle_size_set)
    __swig_setmethods__["enc_size"] = _pyeffort.ezw_header_enc_size_set
    __swig_getmethods__["enc_size"] = _pyeffort.ezw_header_enc_size_get
    if _newclass:enc_size = property(_pyeffort.ezw_header_enc_size_get, _pyeffort.ezw_header_enc_size_set)
    def __init__(self, *args): 
        this = _pyeffort.new_ezw_header(*args)
        try: self.this.append(this)
        except: self.this = this
    def write_out(*args): return _pyeffort.ezw_header_write_out(*args)
    __swig_getmethods__["read_in"] = lambda x: _pyeffort.ezw_header_read_in
    if _newclass:read_in = staticmethod(_pyeffort.ezw_header_read_in)
    def __str__(*args): return _pyeffort.ezw_header___str__(*args)
    __swig_destroy__ = _pyeffort.delete_ezw_header
    __del__ = lambda self : None;
ezw_header_swigregister = _pyeffort.ezw_header_swigregister
ezw_header_swigregister(ezw_header)
ezw_header_read_in = _pyeffort.ezw_header_read_in

import vtk

class EffortData(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, EffortData, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, EffortData, name)
    __repr__ = _swig_repr
    def getHeader(*args): return _pyeffort.EffortData_getHeader(*args)
    def getFilename(*args): return _pyeffort.EffortData_getFilename(*args)
    def getStart(*args): return _pyeffort.EffortData_getStart(*args)
    def getEnd(*args): return _pyeffort.EffortData_getEnd(*args)
    def getType(*args): return _pyeffort.EffortData_getType(*args)
    def getMetric(*args): return _pyeffort.EffortData_getMetric(*args)
    def setApproximationLevel(*args): return _pyeffort.EffortData_setApproximationLevel(*args)
    def load(*args): return _pyeffort.EffortData_load(*args)
    def rows(*args): return _pyeffort.EffortData_rows(*args)
    def cols(*args): return _pyeffort.EffortData_cols(*args)
    def getValue(*args): return _pyeffort.EffortData_getValue(*args)
    def __init__(self, *args): 
        this = _pyeffort.new_EffortData(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _pyeffort.delete_EffortData
    __del__ = lambda self : None;
    def rmse(*args): return _pyeffort.EffortData_rmse(*args)
    def wtrmse(*args): return _pyeffort.EffortData_wtrmse(*args)
    def getVTKEffortData(*args):
        return vtk.vtkObject(str(_pyeffort.EffortData_getVTKEffortData(*args)))


    def __getitem__(self, key):
      r,c = key
      return self.getValue(r,c)

EffortData_swigregister = _pyeffort.EffortData_swigregister
EffortData_swigregister(EffortData)

import source

class FrameId(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, FrameId, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, FrameId, name)
    __repr__ = _swig_repr
    __swig_destroy__ = _pyeffort.delete_FrameId
    __del__ = lambda self : None;
    def __init__(self, *args): 
        this = _pyeffort.new_FrameId(*args)
        try: self.this.append(this)
        except: self.this = this
    def write_out(*args): return _pyeffort.FrameId_write_out(*args)
    __swig_getmethods__["read_in"] = lambda x: _pyeffort.FrameId_read_in
    if _newclass:read_in = staticmethod(_pyeffort.FrameId_read_in)
    __swig_getmethods__["create"] = lambda x: _pyeffort.FrameId_create
    if _newclass:create = staticmethod(_pyeffort.FrameId_create)
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

    def __str__(*args): return _pyeffort.FrameId___str__(*args)
    def __eq__(*args): return _pyeffort.FrameId___eq__(*args)
    def __lt__(*args): return _pyeffort.FrameId___lt__(*args)
    def __gt__(*args): return _pyeffort.FrameId___gt__(*args)
    def module(*args): return _pyeffort.FrameId_module(*args)
    def offset(*args): return _pyeffort.FrameId_offset(*args)
FrameId_swigregister = _pyeffort.FrameId_swigregister
FrameId_swigregister(FrameId)
FrameId_read_in = _pyeffort.FrameId_read_in
FrameId_create = _pyeffort.FrameId_create



