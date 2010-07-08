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
from vtk.util.colors import *

from PyQt4.QtGui import *
from PyQt4.QtCore import *
from vtk.qt4.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor

from numpy import *
from surface import Surface
import icons


#
# Frame to render a single surface.  Provides facilities for selecting 
# part of the surface and displaying the coordinates selected.
#
class RenderFrame(QVTKRenderWindowInteractor):
    def __init__(self, parent):
        QVTKRenderWindowInteractor.__init__(self, parent)

        self.renderer = vtk.vtkRenderer()
        self.GetRenderWindow().AddRenderer(self.renderer)
        
        interactor = vtk.vtkInteractorStyleSwitch()
        self._Iren.SetInteractorStyle(interactor)
                
        self.surface = None

        # Remainng calls set up axes.
        tprop = vtk.vtkTextProperty()
        tprop.SetColor(1,1,1)

        # Create a faint outline to go with the axes.
        self.outline = vtk.vtkOutlineFilter()

        # Initially set up with a box as input.  This will be changed
        # to a plot once the user clicks something.
        self.box = vtk.vtkBox()
        self.box.SetBounds(0,10,0,10,0,10)
        sample = vtk.vtkSampleFunction()
        sample.SetImplicitFunction(self.box)
        sample.SetSampleDimensions(2,2,2)
        sample.SetModelBounds(0,10,0,10,0,5)
        sample.ComputeNormalsOff()

        self.outline.SetInputConnection(sample.GetOutputPort())
        mapOutline = vtk.vtkPolyDataMapper()
        mapOutline.SetInputConnection(self.outline.GetOutputPort())
        self.outlineActor = vtk.vtkActor()
        self.outlineActor.SetMapper(mapOutline)
        self.outlineActor.GetProperty().SetColor(1,1,1)
        self.outlineActor.GetProperty().SetOpacity(.25)
        self.renderer.AddActor(self.outlineActor)

        self.axes = vtk.vtkCubeAxesActor2D()
        self.axes.SetCamera(self.renderer.GetActiveCamera())
        self.axes.SetFlyModeToOuterEdges()

        self.axes.SetLabelFormat("%6.4g")
        self.axes.SetFontFactor(0.8)
        self.axes.SetAxisTitleTextProperty(tprop)
        self.axes.SetAxisLabelTextProperty(tprop)
        self.axes.SetXLabel("MPI Rank")
        self.axes.SetYLabel("Progress")
        self.axes.SetZLabel("Effort")
        self.axes.SetInput(sample.GetOutput())
        self.renderer.AddViewProp(self.axes)

        # Keep original camera around in case it gets changed
        self.originalCamera = self.renderer.GetActiveCamera()

        self.renderer.GetActiveCamera().Pitch(90)     # Want effort to be vertical
        self.renderer.GetActiveCamera().OrthogonalizeViewUp()
        self.renderer.ResetCamera()
        self.renderer.GetActiveCamera().Elevation(15)  # Be slightly above the data

    def setCamera(self, cam):
        self.renderer.SetActiveCamera(cam)

    def getCamera(self):
        return self.renderer.GetActiveCamera()

    def resetCamera(self):
        self.renderer.ResetCamera()

    def getOriginalCamera(self):
        return self.originalCamera

    def setSurface(self, new_surface):
        if self.surface:
            self.renderer.RemoveActor(self.surface.actor())

        self.surface = new_surface
        if not self.surface: 
            return

        self.renderer.AddActor(self.surface.actor())

        ranges = self.surface.bounds()
        self.axes.SetInput(self.surface.vtkOutput())
        self.axes.SetRanges(ranges)
        self.axes.SetUseRanges(True)
        self.outline.SetInput(self.surface.vtkOutput())

        self.renderer.ResetCamera()
        self.update()

    def getRenderer(self):
        return self.renderer

    def contextMenuEvent(self, cmev):
        cmev.accept()


class Dot:
    """Cursor for PickableRenderFrame.  Renders a dot with a translucent
       halo that user can move around the scene.
    """
    def __init__(self):
        # Central dot
        self.dot = vtk.vtkSphereSource()
        self.dot.SetThetaResolution(20)
        self.dot.SetRadius(.5)
        self.dotMapper = vtk.vtkPolyDataMapper()
        self.dotMapper.SetInputConnection(self.dot.GetOutputPort())
        self.dotActor = vtk.vtkActor()
        self.dotActor.SetMapper(self.dotMapper)
        
        # Circular halo around dot
        self.halo = vtk.vtkSphereSource()
        self.halo.SetThetaResolution(20)
        self.halo.SetRadius(2)
        self.haloMapper = vtk.vtkPolyDataMapper()
        self.haloMapper.SetInputConnection(self.halo.GetOutputPort())
        self.haloActor = vtk.vtkActor()
        self.haloActor.SetMapper(self.haloMapper)
        
        self.dotActor.GetProperty().SetColor(red)
        self.haloActor.GetProperty().SetColor(white)
        self.haloActor.GetProperty().SetOpacity(0.1)
        self.haloActor.GetProperty().SetSpecular(0.6)

        self.actor = vtk.vtkAssembly()
        self.actor.AddPart(self.dotActor)
        self.actor.AddPart(self.haloActor)
        
    def SetCenter(self, center):
        self.dot.SetCenter(center)
        self.halo.SetCenter(center)



class PickableRenderFrame(RenderFrame):
    """Extended RenderFrame that supports adding/removing a picker to show
       what point in the data the user is pointing at.
    """
    def __init__(self, parent):
        RenderFrame.__init__(self, parent)
        
        self.picker = vtk.vtkPointPicker()   # Picks points as mouse moves over plot
        self._Iren.SetPicker(self.picker)
        self.dot = Dot()                     # Dot in scene to show what was picked
        self.pickingEnabled = False          # Keep track of whether picking is on

        self.pointUpdateListeners = []

    def pickable(self):
        return self.pickingEnabled

    def setPickable(self, pickable):
        if pickable == self.pickingEnabled:
            return
        
        if pickable:
            self.pickingEnabled = True
            self.observerTag = self.AddObserver('MouseMoveEvent', self.pickSomething)
            self.renderer.AddActor(self.dot.actor)
            self.update()
        else:
            self.pickingEnabled = False
            self.RemoveObserver(self.observerTag)
            self.renderer.RemoveActor(self.dot.actor)
            self.update()

    def addPointUpdateListener(self, pul):
        self.pointUpdateListeners.append(pul)

    def pickSomething(self, iren, evt):
        if not self.surface:
            return

        # Get location of the mouse event from VTK
        x, y = iren.GetEventPosition()

        # Try to pick points in the scene from the mouse position
        if self.picker.Pick(x, y, 0, self.renderer):
            actors = self.picker.GetActors()
            pts = self.picker.GetPickedPositions()

            index = actors.IsItemPresent(self.surface.actor())
            if index:
                point = pts.GetPoint(index-1)

                row = int(round(point[0]))
                col = int(round(point[1]))
                if row < 0: row = 0
                if col < 0: col = 0
                if row >= self.surface.rows(): row = self.surface.rows()-1
                if col >= self.surface.cols(): col = self.surface.cols()-1

                # Get normalized and actual effort values
                effort = self.surface.value(row, col)
                norm_effort = self.surface.normalizedValue(row, col)

                # Update listeners with actual point info
                if self.pointUpdateListeners:
                    for pul in self.pointUpdateListeners:
                        pul.setValue([row, col, effort])

                # Put dot at normalized point on the visualization
                self.dot.SetCenter([row, col, norm_effort])
                self.update()


class PointInfo(QFrame):
    def __init__(self, parent=None):
        QFrame.__init__(self, parent)

        layout = QHBoxLayout()
        layout.setContentsMargins(4,4,4,4)
        layout.setSpacing(4)
        
        class ValueBox(QLineEdit):
            def __init__(self):
                QLabel.__init__(self)
                self.setReadOnly(True)
                self.setAlignment(Qt.AlignRight | Qt.AlignBottom)
                
        layout.addStretch()
        layout.addWidget(QLabel("Rank:"))
        self.rankText = ValueBox()
        layout.addWidget(self.rankText)
        layout.addStretch()

        layout.addWidget(QLabel("Step:"))
        self.stepText = ValueBox()
        layout.addWidget(self.stepText)
        layout.addStretch()

        layout.addWidget(QLabel("Effort:"))
        self.effortText = ValueBox()
        layout.addWidget(self.effortText)
        layout.addStretch()

        self.setLayout(layout)
        self.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Maximum)

    def setValue(self, point):
        self.rankText.setText(str(point[0]))
        self.stepText.setText(str(point[1]))
        self.effortText.setText("%g" % point[2])


class BoundsBox(QFrame):
    def __init__(self, parent, name, width=None):
        QFrame.__init__(self, parent)

        class RangeEdit(QLineEdit):
            def __init__(self, parent, validator):
                QLineEdit.__init__(self, parent)
                self.setValidator(validator)
                self.setAlignment(Qt.AlignRight | Qt.AlignBottom)

        self._validator = QIntValidator(self)
        self._minBox = RangeEdit(self, self._validator)
        self.connect(self._minBox, SIGNAL("editingFinished()"), self.minEditingFinished)
        if width: self._minBox.setFixedWidth(width)

        self._maxBox = RangeEdit(self, self._validator)
        self.connect(self._maxBox, SIGNAL("editingFinished()"), self.maxEditingFinished)
        if width: self._maxBox.setFixedWidth(width)

        self._resetAct = QAction(icons.get("expand"), "Reset", self)
        self._resetAct.setToolTip("Maximize %s Range" % name)
        self.connect(self._resetAct, SIGNAL("triggered()"), self.reset)

        self._maximizeButton = QPushButton(icons.get("expand"), "")
        self._maximizeButton.setFlat(True)
        self._maximizeButton.setContentsMargins(0,0,0,0)
        
        self._changeListeners = []

        layout = QHBoxLayout()
        layout.addWidget(QLabel(name + ":"))
        layout.addWidget(self._minBox)
        layout.addWidget(QLabel("to"))
        layout.addWidget(self._maxBox)
        self.setLayout(layout)

        layout.setContentsMargins(4,4,4,4)
        self.setAllowedRange(0,1)
        self.setRange(0,1)
        self.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Maximum)

    def resetAction(self):
        return self._resetAct

    def reset(self):
        """Reset bounds to max allowable."""
        self._min = self._allowedMin
        self._max = self._allowedMax
        self._minBox.setText(str(self._min))
        self._maxBox.setText(str(self._max - 1))
        self._updateListeners()

    def min(self):
        return self._min

    def max(self):
        return self._max

    def allowedMin(self):
        return self._allowedMin

    def allowedMax(self):
        return self._allowedMax

    def setAllowedRange(self, min, max):
        self._allowedMin = min
        self._allowedMax = max
        self._validator.setRange(min, max-1)

    def putInRange(self, value):
        if value < self._allowedMin:
            return self._allowedMin
        elif value > self._allowedMax:
            return self._allowedMax
        else:
            return value

    def setRange(self, min, max):
        self._min = self.putInRange(min)
        self._max = self.putInRange(max)

        self._minBox.setText(str(self._min))
        self._maxBox.setText(str(self._max - 1))

    def minEditingFinished(self):
        min = int(self._minBox.text())

        if (self._max - min) >= 2:
            self._min = min
        else:
            self._min = self._max - 2
            if self._min < self._allowedMin:
                self._min = self._allowedMin
            self._minBox.setText(str(self._min))

        self._updateListeners()

    def maxEditingFinished(self):
        max = int(self._maxBox.text()) + 1

        if (max - self._min) >= 2:
            self._max = max
        else:
            self._max = self._min + 2
            if self._max > self._allowedMax:
                self._max = self._allowedMax
            self._maxBox.setText(str(self._max - 1))

        self._updateListeners()
        
    def _updateListeners(self):
        for listener in self._changeListeners:
            listener(self._min, self._max)

    def addListenerFunction(self, fun):
        self._changeListeners.append(fun)

    def clearListeners(self):
        del self._changeListeners[:]



class MetricViewer(QFrame):
    def __init__(self, parent=None):
        QFrame.__init__(self, parent)
        self.metric = None
        self.regions = []
        self.renderWavelets = False    # Render wavelets or spatial data
        self.hold = False              # Hold data (ignore updates)
        self.isNew = True              # Whether this has rendered regions yet or not.

        # Allow user to provide a dock so we can set the title.
        self.dock = None
        if isinstance(parent, QDockWidget):
            self.dock = parent
            self.dock.setWidget(self)

        self.setFrameStyle(QFrame.StyledPanel)
        layout = QVBoxLayout()
        layout.setContentsMargins(2,2,2,2)
        layout.setSpacing(0)

        controlbar = QHBoxLayout()
        controlbar.setContentsMargins(0,0,0,0)

        self.toolBar = QToolBar()
        self.createActions()

        self.toolBar.addWidget(QLabel("Metric:"))
        self.metricSelector = QComboBox(self)
        self.metricSelector.setSizeAdjustPolicy(QComboBox.AdjustToContents)
        self.connect(self.metricSelector, SIGNAL("activated(int)"), self.userChangedMetricBox)

        self.toolBar.addWidget(self.metricSelector)
        self.toolBar.addSeparator()
        self.toolBar.addAction(self.waveletAct)
        self.toolBar.addAction(self.pickAct)
        self.toolBar.addAction(self.holdAct)
        self.toolBar.addSeparator()

        self.rankBounds = BoundsBox(self, "Rank", 50)
        self.rankBounds.addListenerFunction(lambda a,b: self.render())
        self.toolBar.addWidget(self.rankBounds)
        self.toolBar.addAction(self.rankBounds.resetAction())

        self.stepBounds = BoundsBox(self, "Step", 50)
        self.stepBounds.addListenerFunction(lambda a,b: self.render())
        self.toolBar.addWidget(self.stepBounds)
        self.toolBar.addAction(self.stepBounds.resetAction())

        controlbar.addWidget(self.toolBar)
        layout.addLayout(controlbar)

        self.pointInfo = PointInfo()
        layout.addWidget(self.pointInfo)
        self.pointInfo.hide()

        # Render frame to show surface for selected metric
        self.renderFrame = PickableRenderFrame(self)
        self.renderFrame.addPointUpdateListener(self.pointInfo)
        layout.addWidget(self.renderFrame)

        self.setLayout(layout)

    def createActions(self):
        self.holdAct = QAction(icons.get("pushpin"), "Pin", self)
        self.holdAct.setToolTip("Pin current plot (ignore selection changes).")
        self.holdAct.setCheckable(True)
        self.holdAct.setChecked(False)
        self.connect(self.holdAct, SIGNAL("toggled(bool)"), self.setHold)
        
        self.waveletAct = QAction(icons.get("multiscale"), "Wavelet", self)
        self.waveletAct.setToolTip("Show wavelet coefficients")
        self.waveletAct.setCheckable(True)
        self.waveletAct.setChecked(False)
        self.connect(self.waveletAct, SIGNAL("toggled(bool)"), self.setRenderWavelets)

        self.pickAct = QAction(icons.get("crosshair"), "Pick", self)
        self.pickAct.setToolTip("Display coordinates for points under mouse.")
        self.pickAct.setCheckable(True)
        self.pickAct.setChecked(False)
        self.connect(self.pickAct, SIGNAL("toggled(bool)"), self.setPickable)

    def setRenderWavelets(self, rw):
        if (rw != self.renderWavelets):
            self.renderWavelets = rw
            self.render()
            
    def setHold(self, hold):
        self.hold = hold

    def setPickable(self, pick):
        self.renderFrame.setPickable(pick)
        if (pick):
            self.pointInfo.show()
        else:
            self.pointInfo.hide()

        
    def setMetric(self, metric):
        if (metric != self.metric):
            self.metric = metric
            self.render()
            if self.dock:
                self.dock.setWindowTitle(str(metric))

    def userChangedMetricBox(self, index):
        self.setMetric(str(self.metricSelector.itemText(index)))

    def __getattr__(self, attr):
        """Makes the MetricViewer behave like a RenderFrame"""
        if hasattr(self.renderFrame, attr):
            return getattr(self.renderFrame, attr)
        else:
            raise AttributeError, self.__class__.__name__ + \
                  " has no attribute named " + attr

    def update(self):
        QFrame.update(self)
        self.renderFrame.update()

    def render(self):
        surface = Surface()
        surface.setRenderWavelets(self.renderWavelets)

        # Only put data for regions that have the relevant metric
        dataList = []
        for r in self.regions:
            if r.hasMetric(self.metric):
                dataList.append(r.dataFor(self.metric))

        # Add to the surface and actually render
        surface.addEffortData(dataList)

        self.rankBounds.setAllowedRange(0, surface.rows())
        self.stepBounds.setAllowedRange(0, surface.cols())
        self.rankBounds.setRange(0, surface.rows())
        self.stepBounds.setRange(0, surface.cols())

        if dataList:
            surface.setRowRange(self.rankBounds.min(), self.rankBounds.max())
            surface.setColRange(self.stepBounds.min(), self.stepBounds.max())

        self.renderFrame.setSurface(surface)

    def setEffortRegions(self, regions):
        # save the container full of regions for use by render()
        self.regions = []
        for r in regions:
            self.regions.append(r)

        # If we don't have a metric set already, grab the first one.
        if not self.metric and self.regions:
            self.metric = self.regions[0].firstMetric()

        # Add any metrics in the selected regions that ARE NOT in the
        # combo box.  Prefer that time's on top.
        metrics = set()
        for r in self.regions:
            metrics.update(r.metrics())

        self.metricSelector.clear()
        if "time" in metrics:  # make time first
            metrics.remove("time")
            self.metricSelector.addItem("time")

        for m in metrics:
            self.metricSelector.addItem(m)

        # Make selection persist.
        mIndex = self.metricSelector.findText(self.metric)

        if mIndex < 0: 
            mIndex = 0
            self.metric = str(self.metricSelector.itemText(mIndex))

        self.metricSelector.setCurrentIndex(mIndex)
        
        self.render()        

#
# Allows viewers in multiple windows to share a camera and be
# updated in realtime.
#
class ViewerCollection:
    def __init__(self):
        self.viewers = []
        self._link = True
        self.src = None

    def setLink(self, link):
        if link != self._link:
            self._link = link
            for viewer in self.viewers:
                if link:
                    self.link(viewer)
                else:
                    self.unlink(viewer)
                    viewer.resetCamera()
                viewer.update()
        
    def getLink(self):
        return self._link
        
    # Add a QVTKRenderWindowInteractor to the collection
    def append(self, viewer):
        self.viewers.append(viewer)
        if self._link:
            self.link(viewer)

    def remove(self, viewer):
        self.viewers.remove(viewer)
        if self._link:
            self.unlink(viewer)

    def link(self, viewer):
        # Listen for render events and share cameras.
        viewer.AddObserver('RenderEvent', self.updateAll)
        if self.viewers:
            cam = self.viewers[0].getCamera()
            viewer.setCamera(cam)

    def unlink(self, viewer):
        # Set viewer back to how it was before.
        viewer.RemoveObservers('RenderEvent')
        viewer.setCamera(viewer.getOriginalCamera())

    def updateAll(self, obj, evt):
        for viewer in self.viewers:
            viewer.update()
        
    def __getitem__(self, index):
        return self.viewers[index]

    def __iter__(self):
        for viewer in self.viewers:
            yield viewer

    def __len__(self):
        return len(self.viewers)
