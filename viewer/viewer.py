
from PyQt4.QtGui import *
from PyQt4.QtCore import *
import sys, os, signal
import vtk, vtkutils
import dendrogram, source, icons
import pyeffort, effort, effort_tree


class Viewer(QMainWindow):
    numClusterings = 0
    numViewers = 0

    def __init__(self):
        QMainWindow.__init__(self, None)
        self.setUnifiedTitleAndToolBarOnMac(True)
        self.setWindowTitle("Libra")
        self.viewers = vtkutils.ViewerCollection()
        self.trees = []
        self.regions = None

        # Initial size/position
        self.resize(1024, 768)
        self.move(50, 50)
        
        # Set up file-based view of effort regions.
        self.fileView = effort_tree.TreeView(self)
        self.fileView.setContextMenuBuilder(self.contextMenuBuilder)
        self.fileView.setDisplayFunction(self.display)
        self.setCentralWidget(self.fileView)

        # Create a dock widget for the source view, initially hidden.
        self.sourceView = source.RegionViewer()
        self.sourceDock = QDockWidget("Source")
        self.sourceDock.setWidget(self.sourceView)
        self.addDockWidget(Qt.RightDockWidgetArea, self.sourceDock)
        self.sourceDock.hide()
        
        # Widgets docked on either side of the main window will go from top to bottom.
        self.setCorner(Qt.TopRightCorner, Qt.RightDockWidgetArea)
        self.setCorner(Qt.BottomRightCorner, Qt.RightDockWidgetArea)
        self.setCorner(Qt.TopLeftCorner, Qt.LeftDockWidgetArea)
        self.setCorner(Qt.BottomLeftCorner, Qt.LeftDockWidgetArea)

        # Set up actions first, then bulid menus out of them.
        self.createActions()
        self.createMenus()

        # Create and attach first viewer window here.        
        self.newViewer()
        self.setDockNestingEnabled(True)
        self.curTree = None

        # Icon for whole application 
        self.setWindowIcon(icons.get("libra_icon128"))
        
        # Open the current directory to begin with
        self.load(os.getcwd())

    def createActions(self):
        # These actions are for the main menus
        self.newViewerAct = QAction(icons.get("file-new"), "&New Viewer", self)
        self.newViewerAct.setShortcut("Ctrl+N")
        self.connect(self.newViewerAct, SIGNAL("triggered()"), self.newViewer)
        
        self.newTerminalAct = QAction(icons.get("terminal"), "New &Terminal", self)
        self.newTerminalAct.setShortcut("Ctrl+T")
        self.connect(self.newViewerAct, SIGNAL("triggered()"), self.newTerminal)
        
        self.openDirAct = QAction(icons.get("folder"), "&Open Effort Directory...", self)
        self.openDirAct.setShortcut("Ctrl+O")
        self.connect(self.openDirAct, SIGNAL("triggered()"), self.open)

        self.quitAct = QAction("&Quit", self)
        self.quitAct.setShortcut("Ctrl+Q")
        self.connect(self.quitAct, SIGNAL("triggered()"), self, SLOT("close()"))

        self.clusterSelectionAct = QAction(icons.get("dendrogram"), "Cluster Selection", self)
        self.clusterSelectionAct.setShortcut("Ctrl+K")
        self.connect(self.clusterSelectionAct, SIGNAL("triggered()"), self.newClustering)

        self.toggleSourceAct = self.sourceDock.toggleViewAction()
        self.toggleSourceAct.setShortcut("Alt+0")
        
        self.linkViewersAct = QAction("Link Viewers", self)
        self.linkViewersAct.setShortcut("Ctrl+L")
        self.linkViewersAct.setCheckable(True)
        self.linkViewersAct.setChecked(self.viewers.getLink())
        self.connect(self.linkViewersAct, SIGNAL("toggled(bool)"), self.viewers.setLink)

        self.aboutAct = QAction(icons.get("libra_icon64"), "About Libra", self)
        self.aboutAct.setShortcut("Ctrl+H")
        self.connect(self.aboutAct, SIGNAL("triggered()"), self.about)

    def newTerminal(self):
        pass

    def createMenus(self):
        """ Builds menu bars out of actions.
            PRE: createActions() has been called. """
        
        self.fileMenu = QMenu("&File", self)
        self.menuBar().addMenu(self.fileMenu);

        self.fileMenu.addAction(self.newViewerAct)
        self.fileMenu.addAction(self.newTerminalAct)
        self.fileMenu.addAction(self.openDirAct)
        self.fileMenu.addSeparator()
        
        self.fileMenu.addAction(self.quitAct)

        self.analysisMenu = QMenu("&Analysis", self)
        self.menuBar().addMenu(self.analysisMenu)
        self.analysisMenu.addAction(self.clusterSelectionAct)

        self.viewMenu = QMenu("&View", self)
        self.menuBar().addMenu(self.viewMenu)
        self.viewMenu.addAction(self.toggleSourceAct)
        self.connect(self.viewMenu, SIGNAL("aboutToShow()"), self.buildViewMenu)

        self.helpMenu = QMenu("&Help", self)
        self.menuBar().addMenu(self.helpMenu)
        self.helpMenu.addAction(self.aboutAct)


    def buildViewMenu(self):
        """ Builds the view menu dynamically from all available dockwidgets."""
        self.viewMenu.clear()

        docks    = filter(lambda x: isinstance(x, QDockWidget), self.children())
        viewers  = filter(lambda x: isinstance(x.widget(), vtkutils.MetricViewer), docks)
        clusters = filter(lambda x: isinstance(x.widget(), dendrogram.TreeView), docks)

        # Builds a submenu of actions or a single menu item out of a list of QDockWidgets
        def addActionSubMenu(menu, name, itemlist):
            if len(itemlist) == 1:
                menu.addAction(itemlist[0].toggleViewAction())
            elif itemlist:
                submenu = menu.addMenu(name)
                for item in itemlist:
                    submenu.addAction(item.toggleViewAction())

        addActionSubMenu(self.viewMenu, "Viewers", viewers)
        addActionSubMenu(self.viewMenu, "Clusters", clusters)

        self.viewMenu.addAction(self.toggleSourceAct)
        self.viewMenu.addSeparator()

        self.viewMenu.addAction(self.linkViewersAct)

    def load(self, directory):
        """Opens a new effort directory and loads it into the fileView.  Right now, the old
           FileView is discarded.
           TODO: figure out some workable scheme for opening multiple experiments at once.
        """
        self.effortDB = effort.DB(4)

        self.effortDB.loadDirectory(directory)
        model = effort_tree.EffortTreeModel(effort_tree.build_from(self.effortDB), self)
        self.fileView.setModel(model)
        self.fileView.expandFirstLevel()
        self.statusBar().showMessage("Opened %s" % os.environ["PWD"])

    def open(self):
        """Prompts the user for a directory where Effort files can be found.  If the user
           selects a directory, opens it up in the viewer via load()
        """
        dirname = QFileDialog.getExistingDirectory(self, "Choose an effort directory.", os.getcwd())
        try:
            # TODO: maybe don't require changing dirs -- this is kind of hacky.b
            os.chdir(dirname)
            source.flushCache()
            self.load(dirname)
        except Exception, e:
            print e
            self.statusBar().showMessage("Couldn't open %s" % dirname)

    def newViewer(self):
        """Creates a new vtkutils.MetricViewer and adds it to the main window.
           If there is a current selection, this will display whatever is in the selection in
           the viewer.

           This tries to create the new viewers in the same part of the main window where
           the old ones are.
        """
        viewerDock = QDockWidget("Metric", self)
        Viewer.numViewers += 1
        if Viewer.numViewers < 10:
            viewerDock.toggleViewAction().setShortcut("Alt+" + str(Viewer.numViewers))
                
        viewer = vtkutils.MetricViewer(viewerDock)

        # Add to the last viewer location if we can, otherwise stick it on top.
        if self.viewers:
            self.addDockWidget(self.dockWidgetArea(self.viewers[0].parent()), viewerDock)
        else:
            self.addDockWidget(Qt.TopDockWidgetArea, viewerDock)
        self.viewers.append(viewer)

        def updateViewerList(on):
            if on:
                if not viewer in self.viewers:
                    self.viewers.append(viewer)
            else:
                self.viewers.remove(viewer)
        self.connect(viewerDock, SIGNAL("visibilityChanged(bool)"), updateViewerList)
        
        selection = self.fileView.getSelectedRegions()
        if selection:
            viewer.setEffortRegions(selection)

    def contextMenuBuilder(self, menu, source):
        """This is a callback from the various TreeViews in the viewer.  It creates a context
           menu so that the user can right-click the selection and cluster what's there.
        """
        self.curTree = source
        menu.addAction(self.clusterSelectionAct)

    def newClustering(self):
        """Looks at the current selection (in the fileView or in an existing clustering)
           and creates a new clustering with the effort regions in the selection.

           Before clustering, this will display a dialog to the user to ask for clustering
           parameters.

           New clustering are created as dendrogram.TreeViews inside of DockWidgets.
           This will try to tabify them on top of an existing clustering, if there is one.
        """
        if not self.curTree:
            return
        
        clusteringName = "Clustering %d" % (Viewer.numClusterings + 1)
        dialog = dendrogram.ClusterParamsDialog(clusteringName)

        dialog.setMetrics(self.curTree.getMetricsInSelection())
        dialog.setDistanceFunctions(["rmse", "wtrmse"])
        dialog.exec_()

        if dialog.accepted:
            Viewer.numClusterings += 1
            dgram = dendrogram.build(self.curTree.getSelectedRegions(),
                                     str(dialog.metric),
                                     getattr(pyeffort.EffortData, str(dialog.distance)))

            dtreeDock = QDockWidget("Effort", self)
            dtreeDock.setWindowTitle(dialog.clusteringName)
            if Viewer.numClusterings < 10:
                dtreeDock.toggleViewAction().setShortcut("Ctrl+" + str(Viewer.numClusterings))
            dtree = dendrogram.TreeView(dgram, dtreeDock)
            dtreeDock.setWidget(dtree)

            # Make the parameters display correctly
            dtree.setContextMenuBuilder(self.contextMenuBuilder)
            dtree.setMetric(str(dialog.metric))
            dtree.setDistanceFunction(str(dialog.distance))
            dtree.setDisplayFunction(self.display)

            # Add tree to the same dock area the last tree ended up, tabifying if needed.
            if self.trees:
                self.tabifyDockWidget(self.trees[0].parent(), dtreeDock)
            else:                                     
                self.addDockWidget(Qt.LeftDockWidgetArea, dtreeDock)

            # Make sure things are added/removed from self.trees when needed.
            self.trees.append(dtree)
            def updateTreeList(on):
                if on:
                    if not dtree in self.trees:
                        self.trees.append(dtree)
                else:
                    self.trees.remove(dtree)
            self.connect(dtreeDock, SIGNAL("visibilityChanged(bool)"), updateTreeList)
            
    def about(self):
        """Displays an about box with contact info for the author."""
        QMessageBox.about(self, "About Libra", \
                          "<h2>Libra</h2>" +
                          "by Todd Gamblin, UNC Chapel Hill." +
                          "<p>" +
                          "Developed in collaboration with Lawrence Livermore National Laboratory." +
                          "<p>" +
                          "Contact <a href=\"mailto:tgamblin@cs.unc.edu\">tgamblin@cs.unc.edu</a> for more information.")

    def display(self, tree):
        """Figures out what's selected and updates the VTK display"""
        self.curTree = tree
        regions = tree.getSelectedRegions()
        if regions and not (self.regions == regions):
            self.regions = regions
            for viewer in self.viewers:
                if not viewer.hold:
                    viewer.setEffortRegions(regions)

        frame = tree.getSelectedFrame()
        if frame:
            self.sourceView.setRegion(frame.region())
            self.sourceView.setFrameIndex(frame.frameIndex())
            self.sourceView.rehighlight()
