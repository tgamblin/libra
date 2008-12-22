import os
from PyQt4.QtGui import *
from PyQt4.QtCore import *

icon_search_path = []

# Appends a directory name to the search path.
# get() will look for icons in this directory.
def addSearchPath(path):
    icon_search_path.append(path)

icon_cache = {}

# Try to find an image for a name.  True on success, 
# false otherwise.
def _searchFor(name):
    for path in icon_search_path:
        for suffix in [".png", ".jpg", ".gif"]:
            image = path + "/" + name + suffix
            if os.path.exists(image):
                icon_cache[name] = QIcon(image)
                return True
    return False

# Get an icon for a suffixless image name.  Looks through
# All directories in the search path to find it.  
# Returns a QIcon
def get(name):
    if not name in icon_cache and not _searchFor(name):
        icon_cache[name] = QIcon()
        print "WARNING: couldn't find icon for " + name

    return icon_cache[name]
