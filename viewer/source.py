#!/usr/bin/python

from PyQt4.QtGui import *
from PyQt4.QtCore import *
import os, StringIO, re, effort, icons

# Map of symtab data.
symtab = None

def flushCache():
    symtab = None

#
# Small class for symbol information from symtab file.
#
class SymData:
    def __init__(self, file, line, fun, module, offset):
        self.module = module
        self.offset = int(offset, 16)
        self.file = file
        self.line = int(line)
        self.fun = fun

    def undefined(self):
        return "?" in (self.module, self.offset, self.file, self.line, self.fun)

#
# Loads symtab data generated by libra-build-viewer-data into
# module-wide map.
#
def loadSymtab(symfile="viewer-data/symtab"):
  global symtab
  if symtab != None:
    return

  # Check for a symtab if we do not have it already
  symtab = {}    # This is the global map of symbols

  # Module mappings encountered while parsing symtab file.  Allows libra-build-viewer-data
  # to override [unknown module] or modules that have been moved.
  modmap = {}    
  try:
    symfile = open(symfile, "r")
    mapping = re.compile(".*=>.*")
    
    for line in symfile:
      line = line.rstrip('\n')
      
      if mapping.match(line):
        original, new = map(str.strip, line.split("=>"))
        modmap[new] = original

      else:
        data = SymData(*line.split("|"))

        # Only add things that contain no unknowns.
        if not data.undefined():
          if data.module in modmap:
            key = (modmap[data.module], data.offset)
          else:
            key = (data.module, data.offset)
          
          symtab[key] = data
      
  except IOError:
    # Just ignore the IO Error.
    pass

def getSymbol(key):
  global symtab
  loadSymtab()
  return symtab.get(key)


def find(targetFile, root="viewer-data/src"):
    for subdirname in os.listdir(root):
        subdir = "%s/%s" % (root,subdirname)
        if os.path.isdir(subdir):
            for filename in os.listdir(subdir):
                if filename == targetFile:
                    file = "%s/%s" % (subdir, filename)
                    return file

class RegionHighlighter(QSyntaxHighlighter):
    def __init__(self, document):
        QSyntaxHighlighter.__init__(self, document)
        self.lineNum = 0
        self.start = 0
        self.end = 0

        self.format = QTextCharFormat()
        self.format.setBackground(QBrush(QColor(210, 210, 255)))
        
    def setLine(self, line):
        self.start = line
        self.end = line
        
    def setRange(self, start, end):
        self.start = start
        self.end = end
        
    def rehighlight(self):
        self.lineNum = 0
        QSyntaxHighlighter.rehighlight(self)

    def highlightBlock(self, text):
        self.lineNum += 1
        if self.start and self.end and self.lineNum in xrange(self.start, self.end+1):
            self.setFormat(0, len(text), self.format)
        
#
# Really simple source viewer with line numbers
#
class SourceViewer(QFrame):
    def __init__(self):
        QFrame.__init__(self)

        textLayout = QHBoxLayout()
        self.numbers = QPlainTextEdit()
        self.text =    QPlainTextEdit()
        self.lines = 0
        self.curLine  = 0

        # Read-only, monospace, no wrapping
        def setupCodeText(textEdit):
            textEdit.setReadOnly(True)
            textEdit.setLineWrapMode(QPlainTextEdit.NoWrap)
            format = QTextCharFormat()
            format.setFontFamily("monaco,luxi mono,courier,monospace")
            format.setFontPointSize(9)
            textEdit.setCurrentCharFormat(format)

            cursor = textEdit.textCursor()
            bfmt = QTextBlockFormat()
            bfmt.setTopMargin(0)
            bfmt.setBottomMargin(0)
            cursor.setBlockFormat(bfmt)
            textEdit.setTextCursor(cursor)

        setupCodeText(self.numbers)
        setupCodeText(self.text)

        # Line numbers view is very small and has no scroll bars.
        self.numbers.setFixedWidth(50)
        self.numbers.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.numbers.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)

        # Link scroll bars for line numbers and main text view.
        self.connect(self.text.verticalScrollBar(), SIGNAL("valueChanged(int)"),
                     self.numbers.verticalScrollBar().setValue)
        self.connect(self.numbers.verticalScrollBar(), SIGNAL("valueChanged(int)"),
                     self.text.verticalScrollBar().setValue)

        textLayout.addWidget(self.numbers)
        textLayout.addWidget(self.text)
        textLayout.setContentsMargins(0,0,0,0)
        textLayout.setSpacing(0)

        self.setLayout(textLayout)

    def loadFile(self, fullpath):
        if not fullpath:
            return
    
        file = open(fullpath)

        stringbuilder = StringIO.StringIO()
        linebuilder = StringIO.StringIO()

        linecount = 1
        for line in file:
            # TODO: maybe figure out a better way to get the highlighting to
            # span the width of the text editor, or use a better text editor.
            justified = line[:-1].ljust(200) + '\n'
            stringbuilder.write(justified)
            linebuilder.write("%5d \n" % linecount)
            linecount += 1

        self.curLine = 0
        self.lines = linecount - 1
        self.text.setPlainText(stringbuilder.getvalue())
        self.numbers.setPlainText(linebuilder.getvalue())

        stringbuilder.close()
        linebuilder.close()
        file.close()

    def setText(self, text):
        self.text.setPlainText(text)

    def goto(self, line):
        cursor = self.text.textCursor()
        distance = line - self.curLine

        if distance > 0:
            direction = QTextCursor.Down
        else:
            direction = QTextCursor.Up

        cursor.movePosition(direction, QTextCursor.MoveAnchor, abs(distance))
        self.text.setTextCursor(cursor)
        self.text.centerCursor()
        
    def __getattr__(self, attr):
        """Delegate to text frame in absence of attributes."""
        if hasattr(self.text, attr):
            return getattr(self.text, attr)
        else:
            raise AttributeError, self.__class__.__name__ + \
                  " has no attribute named " + attr


class CallpathViewer(QFrame):
    def __init__(self, callpathName, regionViewer=None, parent=None):
        QFrame.__init__(self, parent)
        self.path = []
        self.regionViewer = regionViewer

        layout = QVBoxLayout()
        layout.setContentsMargins(0,5,0,0)
        layout.setSpacing(3)

        nameLabel = QLabel(callpathName)        

        self.frameSelect = QComboBox()
        self.frameSelect.setSizeAdjustPolicy(QComboBox.AdjustToContents)
        self.connect(self.frameSelect, SIGNAL("currentIndexChanged(int)"), self.selectionChanged)

        self.topbar = QFrame(self)
        tlayout = QHBoxLayout()
        tlayout.setContentsMargins(0,0,0,0)
        tlayout.setSpacing(5)

        tlayout.addWidget(nameLabel)
        tlayout.addWidget(self.frameSelect)
        tlayout.addStretch()
        self.topbar.setLayout(tlayout)
        layout.addWidget(self.topbar)

        self.viewer = SourceViewer()
        layout.addWidget(self.viewer)
        self.setLayout(layout)

        self._highlighter = RegionHighlighter(self.viewer.document())
        self._highlighter.setRange(5, 20)

    def highlighter(self):
        return self._highlighter

    def addExtraWidget(self, widget):
        self.topbar.layout().addWidget(widget)
        
    def setCallpath(self, callpath):
        self.path = [effort.FrameViewWrapper(frame) for frame in callpath]
        self.frameSelect.clear()

        if not self.path:
            self.path.append(effort.FrameViewWrapper())
            self.frameSelect.addItem("unknown")
        else:
            for frame in self.path:
                self.frameSelect.addItem(frame.prettyLocation())

    def selectedFrame(self):
        index = self.frameSelect.currentIndex()
        if index in xrange(0, len(self.path)):
            return self.path[index]
        else:
            return effort.FrameViewWrapper()

    def setSelectedFrame(self, index):
        self.frameSelect.setCurrentIndex(index)

    def selectionChanged(self, index):
        file = self.selectedFrame().file()
        if not file:
            self.viewer.setText('Unknown file.')
        else:
            fullpath = find(file)
            if not fullpath:
                self.viewer.setText("Could not find file '%s'." % file)
            else:
                self.viewer.loadFile(fullpath)
                self.viewer.goto(self.selectedFrame().line())

                # If we're part of a region viewer, have it do the highlighting
                # otherwise do it ourselves since we only care about our own
                # callpath.
                if self.regionViewer:
                    self.regionViewer.rehighlight()
                else:
                    self._highlighter.setLine(self.selectedFrame().line())
                    self.rehighlight()

class RegionViewer(QFrame):
    def __init__(self, parent=None):
        QFrame.__init__(self, parent)
        self.region = None
        
        layout = QVBoxLayout()
        layout.setContentsMargins(0,0,0,0)
        layout.setSpacing(0)

        # Vertical splitter for start and end callpath viewers
        self.splitter = QSplitter(Qt.Vertical)
        self.startView = CallpathViewer("Start:", self)
        self.endView = CallpathViewer("End:", self)
        self.splitter.addWidget(self.startView)
        self.splitter.addWidget(self.endView)
        self.splitter.setChildrenCollapsible(False)

        # Add an extra button to the end viewer to hide it
        self.hideButton = QPushButton()
        self.hideButton.setFlat(True)
        self.hideButton.setIcon(icons.get("down_triangle"))
        self.hideButton.setText("Hide End")
        self.connect(self.hideButton, SIGNAL("clicked()"), self.toggleExpand)
        self.endView.addExtraWidget(self.hideButton)

        # Button to go below the splitter to show end when it's hidden
        self.showButton = QPushButton()
        self.showButton.setFlat(True)
        self.showButton.setIcon(icons.get("up_triangle"))
        self.showButton.setText("Show End")
        self.connect(self.showButton, SIGNAL("clicked()"), self.toggleExpand)
        self.showButton.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Maximum)
        showlayout = QHBoxLayout()
        showlayout.addStretch()
        showlayout.addWidget(self.showButton)

        # Put the main layout together here.
        layout.addWidget(self.splitter)
        layout.addLayout(showlayout)
        self.setLayout(layout)

        # Start things out with the end hidden.
        self.expanded = True
        self.toggleExpand()

    def toggleExpand(self):
        if self.expanded:
            self.showButton.show()
            self.endView.hide()
        else:
            self.showButton.hide()
            self.endView.show()

        self.expanded = not self.expanded
            
    def setRegion(self, region):
        self.region = region
        self.startView.setCallpath(region.start())
        self.endView.setCallpath(region.end())

    def setFrameIndex(self, index):
        self.startView.setSelectedFrame(index)
        self.endView.setSelectedFrame(index)

    def rehighlight(self):
        """ Quick hack for highlighting code regions in the same file.
            This won't work in all cases.
            TODO: Use ROSE for proper callpath intervals.
        """
        startFrame = self.startView.selectedFrame()
        endFrame = self.endView.selectedFrame()
        if startFrame.file() == endFrame.file():
            self.startView.highlighter().setRange(startFrame.line(), endFrame.line())
            self.endView.highlighter().setRange(startFrame.line(), endFrame.line())
        else:
            self.startView.highlighter().setLine(startFrame.line())
            self.endView.highlighter().setLine(endFrame.line())
    
        self.startView.highlighter().rehighlight()
        self.endView.highlighter().rehighlight()

    def sizeHint(self):
        return QSize(500, 900)
