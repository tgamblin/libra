
from PyQt4.QtGui import *
from PyQt4.QtCore import *
from hcluster import *
from numpy import *
import effort_tree


class Dendrogram:
    def __init__(self, root, linkage):
        self.root = root
        self.linkage = linkage
    
    def make_cluster_view(self, parent, trunc=0):
        self.root.make_cluster_view(parent, trunc)

#
# Simple class for node in dendrogram.  Provides instance methods
# for building various QTreeWidget visualizations.
#
class Node:
    def __init__(self, region, l=None, r=None):
        self.region = region
        self.l = l
        self.r = r

    def leaf(self):
        return not self.l and not self.r
    
    #
    # Recursively builds a tree of effort tree nodes from the dendrogram.
    # Can optionally truncate at an arbitrary level in the tree.
    #
    # Params:
    # parent     effort_tree.Node to attach the resulting effort tree to.
    # trunc      optional level at which to truncate the dendrogram
    # level      internal parameter to keep track of level during recursion
    #
    def make_cluster_view(self, parent, trunc=0, level=0):
        if self.region:
            node = effort_tree.Region(self.region, parent)
        elif trunc and level >= trunc:
            node = parent  # Collapse tree below truncation level
        else:
            node = effort_tree.Node(str(level), parent)

        if not self.leaf():
            self.l.make_cluster_view(node, trunc, level+1)
            self.r.make_cluster_view(node, trunc, level+1)

    #
    # Returns a flat clustering resulting from cutting the dendrogram at
    # a particular level.  Supplying 0 for the truncation level results
    # in no truncation.
    #
    def flatten(self, trunc=0, level=0):
        if self.leaf():
            return [[self.region]]
        else:
            sublists = self.l.flatten(trunc, level+1) + self.r.flatten(trunc, level+1)
            if trunc and level >= trunc:
                return [reduce(lambda x,y:x+y, sublists)]
            else:
                return sublists


    def output(self, level=0):
        for i in xrange(0,level):
            print "  ",
            
        if self.leaf():
            print self.region
        else:
            print level
            self.l.output(level+1)
            self.r.output(level+1)
        
    # Get the depth of this dendrogram.  Implies a search
    def depth(self):
        if self.leaf(): return 1
        else: return 1 + max(self.l.depth(), self.r.depth())


#
# Applies hierarchical clustering to a set of effort regions and returns
# a dendrogram of the clustering.
#
def build(region_container, metric, distance):
    regions = [r for r in region_container]

    # Compute square distance matrix for regions and convert to condensed
    # Then do hierarchical clustering on the data
    n = len(regions)
    if n > 1:
        sd = zeros([n,n])
        ss = zeros([n,n])
        for i in arange(0,n):
            for j in arange(i,n):
                if i == j:
                    sd[i,j] = 0 # Force zeros to avoid small error
                else:
                    ldata = regions[i].dataFor(metric)
                    rdata = regions[j].dataFor(metric)
                    sd[i,j] = sd[j,i] = distance(ldata, rdata)
        d = squareform(sd)

        Z = linkage(d, 'complete')
    else:
        Z = []
        
    # Can't build QListViewItems bottom-up, so build our own tree
    # bottom-up first.
    nodes = [Node(region) for region in regions]
    for row in Z:
        # Construct internal nodes with no name
        l = int(row[0])
        r = int(row[1])
        nodes.append(Node(None, nodes[l], nodes[r]))

    # Return the dendrogram we just constructed
    return Dendrogram(nodes[-1], Z)


#
# Special widget for viewing dendrograms.  Includes a tree view but also
# a slider for adjusting dendrogram truncation to taste.
#
class TreeView(QWidget):
    def __init__(self, dendro, parent=None):
        QWidget.__init__(self, parent)

        layout = QVBoxLayout()
        layout.setContentsMargins(0,0,0,0)

        # Set up some useful defaults for viewing.
        self.tree = effort_tree.TreeView(self)
        layout.addWidget(self.tree)

        # Keeep this for later in case we rebuild.
        self.dendro = dendro
        self.trunc = 4        # initial truncation of dendrogram
        
        # Hook up a slider at the bottom so that we can adjust
        # the truncation level of the dendrogram
        self.depth = dendro.depth()

        bottomBox = QWidget()
        blayout = QGridLayout()
        blayout.setContentsMargins(0,0,0,0)
        blayout.addWidget(QLabel("Metric"), 0, 0)
        blayout.addWidget(QLabel("Distance Function:"), 1, 0)

        self.metricLabel = QLabel("")
        blayout.addWidget(self.metricLabel, 0, 1)
        self.distanceLabel = QLabel("")
        blayout.addWidget(self.distanceLabel, 1, 1)

        slayout = QHBoxLayout()
        slayout.setContentsMargins(2,2,2,2)
        slayout.addWidget(QLabel("Truncate:"))

        self.slider = QSlider(Qt.Horizontal)
        self.slider.setRange(1, self.depth)
        self.slider.setSliderPosition(1)
        self.slider.setSingleStep(1)
        self.slider.setTickInterval(1)
        self.slider.setTickPosition(QSlider.TicksAbove)
        slayout.addWidget(self.slider)
        
        self.slider_label = QLabel("")
        slayout.addWidget(self.slider_label)

        blayout.addLayout(slayout, 2, 0, 1, 2)
        bottomBox.setLayout(blayout)

        layout.addWidget(bottomBox)
        self.setLayout(layout)
        
        # Construct initial cluster tree, with a named root node.
        self.rebuild(self.trunc)
        self.slider.setValue(self.trunc)  # Set initial slider value 

        # Hook this up last so setValue() doesn't trigger rebuild
        self.connect(self.slider, SIGNAL("valueChanged( int )"), self.rebuild)

    # Behave like our contained TreeView.
    def __getattr__(self, attr):
        if hasattr(self.tree, attr):
            return getattr(self.tree, attr)
        else:
            raise AttributeError, self.__class__.__name__ + \
                  " has no attribute named " + attr

    def setMetric(self, metric):
        self.metricLabel.setText(metric)

    def setDistanceFunction(self, distanceFunction):
        self.distanceLabel.setText(distanceFunction)

    def selectedIndexes(self):
        return [i.internalPointer() for i in self.tree.selectedIndexes()]

    def truncation_level(self):
        return trunc
    
    def rebuild(self, level=0):
        self.trunc = min(level, self.depth)
        self.slider_label.setText(str(self.trunc))

        root = effort_tree.Node("Clustered")
        self.dendro.make_cluster_view(root, self.trunc)

        model = effort_tree.EffortTreeModel(root, self)
        self.tree.setModel(model)

        for index in model.internalNodes(self.trunc):
            self.tree.expand(index)


class ClusterParamsDialog(QDialog):
    def __init__(self, clusteringName="Clustering", parent=None):
        QDialog.__init__(self, parent)
        self.accepted = False

        self.setWindowTitle("Cluster Selection")
        self.label = QLabel("Cluster selection with:")

        self.metricLabel = QLabel("Metric: ")
        self.distanceLabel = QLabel("Distance function: ")

        self.metricChooser = QComboBox()
        self.distanceChooser = QComboBox()

        self.lineEdit = QLineEdit(clusteringName)
        self.connect(self.lineEdit, SIGNAL("returnPressed()"), self.ok)
        self.lineEdit.setSelection(0, len(clusteringName))

        self.cancelButton = QPushButton("Cancel")
        self.connect(self.cancelButton, SIGNAL("clicked()"), self.cancel)

        self.okButton = QPushButton("OK")
        self.connect(self.okButton, SIGNAL("clicked()"), self.ok)
        self.okButton.setDefault(True)
        
        layout = QVBoxLayout()
        layout.addWidget(self.label)

        chooseLayout = QHBoxLayout()
        layout.addLayout(chooseLayout)
        chooseLayout.addWidget(self.metricLabel)
        chooseLayout.addWidget(self.metricChooser)
        chooseLayout.addWidget(self.distanceLabel)
        chooseLayout.addWidget(self.distanceChooser)

        layout.addWidget(QLabel("Enter a name for this clustering:"))
        layout.addWidget(self.lineEdit)

        buttonLayout = QHBoxLayout()
        layout.addLayout(buttonLayout)
        buttonLayout.addStretch()
        buttonLayout.addWidget(self.cancelButton)
        buttonLayout.addWidget(self.okButton)

        self.setLayout(layout)

    def ok(self):
        self.accepted = True
        self.metric = self.metricChooser.itemText(self.metricChooser.currentIndex())
        self.distance = self.distanceChooser.itemText(self.distanceChooser.currentIndex())
        self.clusteringName = self.lineEdit.displayText()
        self.close()

    def cancel(self):
        self.accepted = False
        self.close()

    def setMetrics(self, list):
        self.metricChooser.addItems(list)
        
    def setDistanceFunctions(self, funs):
        self.distanceChooser.addItems(funs)

