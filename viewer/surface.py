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
