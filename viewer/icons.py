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
