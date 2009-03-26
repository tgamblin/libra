#
# This file contains routines for building QListViews of effort data.
# QListViews are arranged in a tree, either by metric, effort type, and
# callpath, or by hierarchy determined by clustering.
#
from PyQt4.QtGui import *
from PyQt4.QtCore import *
import icons, operator, re, effort

#
# Internal node of an effort tree.  Just a named directory, but
# has facilities to get all Regions and/or selected items
# beneath.  These can be selected as representatives of all
# their children.
#
class Node:
    def __init__(self, name="", parent=None):
        self._children = []
        self._name = str(name)
        if parent:
            parent.addChild(self)
        else:
            self._parent = None

    def addChild(self, child):
        assert not (child in self._children)
        self._children.append(child)
        child._parent = self

    def name(self):
        return self._name

    def getAllRegions(self):
        return reduce(lambda x,y: x+y,
                      [child.getAllRegions() for child in self.children()])

    def children(self):
        for c in self._children: yield c

    def childCount(self):
        return len(self._children)

    def child(self, index):
        return self._children[index]

    def root(self):
        if self._parent:
            return self._parent
        else:
            return self

    def parent(self):
        return self._parent

    def index(self):
        if self._parent:
            return self._parent._children.index(self)
        else:
            return 0

    def sort(self):
        # Sort children by name.
        self._children.sort(key=lambda x:x.sortKey())

    def sortKey(self):
        return self._name
        
    def icon(self):
        return icons.get("folder")

    def isRegion(self):
        return False

    def isFrame(self):
        return False


#
# For children of regions;  these are just for displaying all
# callframes of start and end paths.
#
class FrameNode(Node):
    def __init__(self, start, end, index, data, all_total, parent=None):
        Node.__init__(self, index, parent)
        self._start = effort.FrameViewWrapper.fromIndex(start, index)
        self._end = effort.FrameViewWrapper.fromIndex(end, index)
        self._data = data
        self._all_total = float(all_total)

    def data(self, index):
        return {
            0: self._start.prettyLocation(),
            1: self._end.prettyLocation(),
            2: self.total(),
            3: self.meanVariance(),
            4: self.meanRowSkew(),
            5: self.meanRowKurtosis(),
#            2: self._start.prettyModule(),  
#            3: self._end.prettyModule()
            }.get(index)

    def tip(self, index):
        return {
            2: self._start.prettyModule(True),
            3: self._end.prettyModule(True)
            }.get(index)
    

    def total(self):
        if not self.parent():
            total = self._data.total()
            return "%g (%.2f%%)" % (total, (100.0 * total / self._all_total))
        else:
            return None
    
    def meanVariance(self):
        if not self.parent():
            return self._data.meanVariance()
        else:
            return None

    def meanRowSkew(self):
        if not self.parent():
            return self._data.meanRowSkew()
        else:
            return None

    def meanRowKurtosis(self):
        if not self.parent():
            return self._data.meanRowKurtosis()
        else:
            return None

    def getAllRegions(self):
        return []

    def startFrame(self):
        return self._start
        
    def endFrame(self):
        return self._end
        
    def icon(self):
        return None

    def name(self):
        return "%s -> %s" % (self._start.name(), self._end.name())

    def isRegion(self):
        return False

    def isFrame(self):
        return True

    def region(self):
        return self.parent().region()

    def frameIndex(self):
        return self.index() + 1


#
# Node in tree representing the data from a single effort file.
# Select one of these to plot just the effort from that file.
#
class Region(Node):
    def __init__(self, region, all_total, parent=None):
        start = region.start()
        end = region.end()
        self._region = region
        self._firstFrame = FrameNode(start, end, 0, self._region.dataFor("time"), all_total)
        Node.__init__(self, self._firstFrame.name(), parent)

        maxpath = max(len(start), len(end))
        for i in xrange(1, maxpath):
            self.addChild(FrameNode(start, end, i, self._region.dataFor("time"), 0))
        
    def sortKey(self):
        return -self.region().dataFor("time").total()

    def data(self, index):
        return self._firstFrame.data(index)
        
    def tip(self, index):
        return self._firstFrame.data(index)
        
    def startFrame(self):
        return self._firstFrame.startFrame()
            
    def endFrame(self):
        return self._firstFrame.endFrame()
            
    def region(self):
        return self._region

    def getAllRegions(self):
        return [self._region]

    def icon(self):
        return icons.get("grid")

    def isRegion(self):
        return True

    def isFrame(self):
        return True

    def frameIndex(self):
        return 0


#
# Builds a tree with effort regions at the leaves and internal
# nodes for each phase of execution.
#
def build_from(regionsDB):
    root = Node("All")
    phases = {}

    # total cpu time (for all procs) in nanoseconds
    all_total = float(regionsDB.totalTime * 1e9 * regionsDB.vprocs)

    for region in regionsDB:
        id = str(region.type())
        if not id in phases:
            phases[id] = Node(id, root)
        phases[id].addChild(Region(region, all_total))
            
    root.sort()

    for child in root.children():
        child.sort()

    return root


#
# Model maps trees of Nodes and regions to something displayable by Qt.
#
class EffortTreeModel(QAbstractItemModel):
#    columns = ["Start", "End", "Start Module", "End Module"]
    columns = ["Start", "End", "TotalTime", "Variance", "MeanSkew", "MeanKurtosis"]

    # Set up  tree model with at least a root.  User can provide
    # Their own root, too.
    def __init__(self, root=None, parent=None):
        QAbstractItemModel.__init__(self, parent)
        self.rootItem = Node("Effort")
        if root:
            self.rootItem.addChild(root)

            
    # Columns are set up in columns array.  This just returns its length.
    def columnCount(self, parent=QModelIndex()):
        return len(EffortTreeModel.columns)

    # Returns child count of the Node that corresponds to parent.
    def rowCount(self, parent=QModelIndex()):
        # Only as many columns as there are headers
        if parent.column() > len(EffortTreeModel.columns):
            return 0

        if not parent.isValid():
            parentItem = self.rootItem
        else:
            parentItem = parent.internalPointer()

        return parentItem.childCount()

    # Returns an index created for the parent of the Node
    # corresponding to index.
    def parent(self, index):
        if not index.isValid():
            return QModelIndex()

        childItem = index.internalPointer()
        parentItem = childItem.parent()

        if parentItem == self.rootItem:
            return QModelIndex()
        else:
           return self.createIndex(parentItem.index(), 0, parentItem)

    # Returns an index created for the (row,col)th child of parent.
    def index(self, row, col, parent):
        if not row in xrange(0, self.rowCount(parent)) or \
           not col in xrange(0, self.columnCount(parent)):
            return QModelIndex()

        if not parent.isValid():
            parentItem = self.rootItem
        else:
            parentItem = parent.internalPointer()

        childItem = parentItem.child(row)
        if childItem:
            return self.createIndex(row, col, childItem)
        else:
            return QModelIndex()

    def allIndices(self, maxdepth=-1, level=0, index=QModelIndex()):
        if maxdepth == level:
            return
        
        row = 0
        node = self.index(row, 0, index)

        # Iterate over children of index.
        while node != QModelIndex(): 
            # Yield all children first.
            for child in self.allIndices(maxdepth, level+1, node):
                yield child
            
            # Yield each node after all its children have been yielded
            yield node
            node = self.index(row, 0, index)
            row += 1

    def internalNodes(self, maxdepth=-1, level=0, index=QModelIndex()):
        for node in self.allIndices(maxdepth, level):
            if not node.internalPointer().isFrame(): 
                yield node
            
    # Flags for all items in the model.
    def flags(self, index):
        if not index.isValid():
            return Qt.ItemIsEnabled
        
        return Qt.ItemIsEnabled | Qt.ItemIsSelectable

    # Returns name for a particular column, straight from the columns array.
    def headerData(self, section, orientation, role):
        if orientation == Qt.Horizontal and role == Qt.DisplayRole:
            return QVariant(EffortTreeModel.columns[section])
        else:
            return QVariant()
    
    # Returns data from Nodes and regions, and icons for nodes/regions
    # if appropriate.
    def data(self, index, role):
        if not index.isValid():
            return QVariant()

        node = index.internalPointer()
        if role == Qt.DecorationRole and index.column() == 0:
            return QVariant(node.icon())

        elif role == Qt.DisplayRole:
            if node.isFrame():
                return QVariant(node.data(index.column()))
            else:
                if index.column() == 0:
                    return QVariant(node.name())

        elif role == Qt.ToolTipRole and node.isFrame():
            return QVariant(node.tip(index.column))

        return QVariant()


#
# Tree View for effort regions.  
#
class TreeView(QTreeView):
    # TODO: do this better. This is hard coded for 1024-wide window.
    widths = [375, 375, 170, 125, 125, 125]

    def __init__(self, parent=None):
        QTreeView.__init__(self, parent)
        self.setModel(EffortTreeModel(None, self))
        self.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.setAlternatingRowColors(True)
        self.display = self.doNothing
        for i in xrange(0, len(self.widths)):
            self.setColumnWidth(i, self.widths[i])

        self.setIndentation(12)
        self.contextMenuBuilder = None
    
    def expandFirstLevel(self):
        # expand first level of tree
        childCount = self.model().rowCount(self.rootIndex())
        for r in xrange(0, childCount):
            self.expand(self.model().index(r, 0, self.rootIndex()))
        
    def setDisplayFunction(self, fun):
        self.display = fun

    def doNothing(self, *args):
        pass

    def selectionChanged(self, selected, deselected):
        self.display(self)

    # Returns the selected regions in this tree viewthe tab currently displayed
    def getSelectedRegions(self):
        regions = set()
        nodes =  [i.internalPointer() for i in self.selectedIndexes()]
        for node in nodes:
            regions.update(node.getAllRegions())
        return regions

    def getSelectedFrame(self):
        """Returns the currently selected Region or Frame in the tree.  If multiple
           are selected, returns the first.
        """
        indices = self.selectedIndexes()
        if not indices: return None

        selectedNode = None
        for node in [i.internalPointer() for i in indices]:
            if node.isFrame():
                selectedNode = node
                break

        return selectedNode
        
    def setContextMenuBuilder(self, builder):
        self.contextMenuBuilder = builder

    def contextMenuEvent(self, cmev):
        if self.contextMenuBuilder:
            contextMenu = QMenu(self)
            self.contextMenuBuilder(contextMenu, self)
            contextMenu.exec_(QCursor.pos())
        
    def getMetricsInSelection(self):
        metrics = set()
        for r in self.getSelectedRegions():
            metrics.update(r.metrics())

        list = []
        if "time" in metrics:
            metrics.remove("time")
            list.append("time")
        for m in metrics:
            list.append(m)
        return list

