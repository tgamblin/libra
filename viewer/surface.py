#################################################################################################
# Copyright (c) 2010, Lawrence Livermore National Security, LLC.  
# Produced at the Lawrence Livermore National Laboratory  
# Written by Todd Gamblin, tgamblin@llnl.gov.
# LLNL-CODE-417602
# All rights reserved.  
# 
# This file is part of Libra. For details, see http://github.com/tgamblin/libra.
# Please also read the LICENSE file for further information.
# 
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
# 
#  * Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the disclaimer below.
#  * Redistributions in binary form must reproduce the above copyright notice, this list of
#    conditions and the disclaimer (as noted below) in the documentation and/or other materials
#    provided with the distribution.
#  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
#    or promote products derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
# LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#################################################################################################
import vtk
import vtk.util
from numpy import *
from libvtkeffort import *
import effort


class Surface:
    """Class for combining multiple vtkEffortData together into a single surface.
       Pieces of VTK pipeline are built and resulting actor is acessible via
       'actor' attribute.
    """
    def __init__(self):
        """Builds basic pieces of pipeline, but doesn't add any data.
           Use addEffortData() to add data to the surface before rendering.
        """
        self._fun = vtkEffortSource()

        self._zscalars = vtkZValueScalars()
        self._zscalars.SetInputConnection(self._fun.GetOutputPort())

        self._normals = vtk.vtkPolyDataNormals()
        self._normals.SetInputConnection(self._zscalars.GetOutputPort())
        self._normals.SetFeatureAngle(90)

        self._mapper = vtk.vtkPolyDataMapper()
        self._mapper.SetInputConnection(self._normals.GetOutputPort())
        self._mapper.ScalarVisibilityOn()
    
        self._actor = vtk.vtkActor()
        self._actor.SetMapper(self._mapper)

    def addEffortData(self, list_of_effort_data):
        """Invokes addEffortData on underlying vtkEffortSource and 
           updates other display properties.
        """
        # Add all effort to the vtkEffortFunction
        for e in list_of_effort_data:
            self._fun.AddEffort(e.getVTKEffortData())

        # set color table according to new data.
        bounds = self._fun.GetBounds()

        lut = vtk.vtkLookupTable()
        lut.SetTableRange(bounds[4:6])
        lut.SetHueRange(0.667, .667)
        lut.SetSaturationRange(1, .3)
        lut.Build()
        self._mapper.SetLookupTable(lut)

    def setRowRange(self, min, max):
        self._fun.SetRowRange(min, max)

    def setColRange(self, min, max):
        self._fun.SetColRange(min, max)

    def rows(self):
        return self._fun.Rows()

    def cols(self):
        return self._fun.Cols()

    def value(self, row, col):
        """Fetch the value at (row, col) in the internal matrix"""
        return self._fun.GetValue(row, col)

    def normalizedValue(self, row, col):
        """Fetch the normalized value at (row, col) in the internal matrix"""
        return self._fun.GetNormalizedValue(row, col)

    def setRenderWavelets(self, render):
        self._fun.SetRenderWavelets(render)

    def setColor(self, r,g,b):
        self._actor.GetProperty().SetColor(r,g,b)

    def bounds(self):
        return self._fun.GetBounds()

    def actor(self):
        return self._actor
        
    def normalizedBounds(self):
        return self._fun.GetNormalizedBounds()
        
    def vtkOutput(self):
        return self._zscalars.GetOutput()
