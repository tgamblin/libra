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

#
#  LX_PYTHON_DEVEL
#  ------------------------------------------------------------------------
#  Adds --with-python= and --with-python-version= to configure and searches
#  for a python binary, Python.h, and a python lib directory.  Properly 
#  handles paths to Mac OS X frameworks as well.
# 
#  If the python interpreter is foubd, this will set up the following directories 
#  identically to the way the builtin AM_PATH_PYTHON would do it:
# 
#     PYTHON               Python interpreter
#     PYTHON_VERSION       Version of python requested and/or detected.
#     PYTHON_PREFIX        The string '${prefix}'.
#     PYTHON_EXEC_PREFIX   The string '${exec_prefix}'.
#     PYTHON_PLATFORM
#
#  This will AC_SUBST the following devel variables for use in Makefile.am: 
#
#     PYTHON_CPPFLAGS      Include flags for dir containing Python.h
#     PYTHON_LDFLAGS       Linker flags for either:
#                          - Linking against libpython<PYTHON_VERSION>.so on linux
#                          - Linking against the Python Framework on Mac OS X
#     PYTHON_RPATH         Rpath arguments for the python directory.
#
#  Finally, this sets some variables you can use in configure.ac:
#     have_python          yes if interpreter was found, no otherwise.
#     have_python_devel    yes if dev libs and headers were found, no otherwise.
# 
AC_DEFUN([LX_PYTHON_DEVEL],
[
AC_ARG_WITH([python-version],
            AS_HELP_STRING([--with-python-version], [Specify the version of python to use.  If not supplied, defaults to the highest found.]),
            [PYTHON_VERSION="$withval"])

echo "checking for python..."

AC_ARG_WITH([python],
    AS_HELP_STRING([--with-python], [Specify the location of your python installation.  This can either be a GNU standard install directory or a path to a Mac OS X .framework directory.]),
    [
     # If the user supplied --with-python:
     if [[ ! -d "$withval" ]]; then
         AC_MSG_ERROR(["$withval" is not a valid directory.])
     fi
     pyhome="$withval"

     # Need this so that we can check for frameworks properly.
     echo $ECHO_N "checking if Python is a Mac OS X Framework... $ECHO_C"
     SYS_TYPE=`uname -s`
     if test "x$SYS_TYPE" = "xDarwin" && (echo $pyhome | grep -q '\.framework$'); then
         echo yes
         py_is_framework=yes
     else
         echo no
         py_is_framework=no
     fi

     if [[ "x$py_is_framework" = xyes ]]; then
         # Mac OS X Frameworks have binaries in particular version directories.
         if [[ "x$PYTHON_VERSION" = "x" ]]; then
             PYTHON="$pyhome/Versions/Current/bin/python"
         else 
             PYTHON="$pyhome/Versions/${PYTHON_VERSION}/bin/python${PYTHON_VERSION}"
         fi
     else
         PYTHON="$pyhome/bin/python${PYTHON_VERSION}"
     fi

     if [[ ! -x "$PYTHON" ]]; then
         have_python=no
         AC_MSG_ERROR([Could not find python in $withval.])
     else  
         have_python=yes
     fi
    ],
    [
     # If the user didn't supply --with-python:
     if [[ "x$PYTHON_VERSION" != "x" ]]; then
          # User supplied a version they want for python: fail if we can't find the exact version.
          AC_PATH_PROGS(PYTHON, [python${PYTHON_VERSION} python])
          if [[ "x$PYTHON" == x ]]; then
              AC_MSG_ERROR([Could not find python${PYTHON_VERSION}.])
              have_python=no
          else
              actualversion=`${PYTHON} -V 2>&1 | sed 's_^[[A-Za-z]]* \([[0-9]]\.[[0-9]]\).*$_\1_'`
              if [[ "x$actualversion" != "x$PYTHON_VERSION" ]]; then
                  have_python=no
                  AC_MSG_ERROR([Python version $PYTHON_VERSION not found.])
              else
                  have_python=yes
              fi
          fi
      else 
          # No exact version specified. Try to find python, and just omit it from the buld if not found.
          AC_PATH_PROGS(PYTHON, [python python2.5 python2.4 python2.3])
          if [[ "x$PYTHON" != x ]]; then
              have_python=yes
          else
              AC_MSG_NOTICE([Could not find python interpreter.])
              have_python=no
          fi
      fi

      if [[ "x$have_python" = xyes ]]; then
          # Infer python home directory from the executable's location, as it was autodetected
          pyhome=`echo ${PYTHON} | sed 's_/bin/[[^/]]*\$__'`
          AC_MSG_NOTICE([Assuming python home directory is "$pyhome".])
          if [[ "x$SYS_TYPE" = "xDarwin" ]]; then
              echo "  NOTE: On Mac OS X, you may need to explicitly supply"
              echo "        the path to Python.framework on your system."
          fi
      fi
    ])

 AC_SUBST(PYTHON)
 if [[ have_python != xno -a "x${PYTHON_VERSION}" = x ]]; then
     # Make sure the version is set before continuing.
     PYTHON_VERSION=`${PYTHON} -V 2>&1 | sed 's_^[[A-Za-z]]* \([[0-9]]\.[[0-9]]\).*$_\1_'`
 fi
 AC_SUBST(PYTHON_VERSION)

 # Should have pyhome and PYTHON_VERSION set here.  Now test for libs, etc. and executables.
 if [[ "x$have_python" = xyes ]]; then
     echo "found Python interpreter: ${PYTHON}."

     # First set up all the variables that AM_PATH_PYTHON does.
     AC_SUBST(PYTHON_PREFIX)
     PYTHON_PREFIX='${prefix}'

     AC_SUBST(PYTHON_EXEC_PREFIX)
     PYTHON_EXEC_PREFIX='${exec_prefix}'
     
     AC_SUBST(PYTHON_PLATFORM)
     PYTHON_PLATFORM=`$PYTHON -c "import sys; print sys.platform"`
                              
     AC_SUBST(pythondir)
     pythondir="${PYTHON_PREFIX}/lib/python${PYTHON_VERSION}/site-packages"
     
     AC_SUBST(pkgpythondir)
     pkgpythondir="${pkgpythondir}/${PACKAGE}"
     
     AC_SUBST(pyexecdir)
     pyexecdir=$pythondir
     
     AC_SUBST(pkgpyexecdir)
     pkgpyexecdir=$pkgpythondir
     
     # Then set up devel flags either for a framework or for standard GNU install
     if [[ "x$py_is_framework" = xyes ]]; then
         PYTHON_FDIR=`echo ${pyhome} | sed 's_/[[^/]]*\.framework$__'`

         PYTHON_CPPFLAGS="-I${pyhome}/Headers"
         PYTHON_LDFLAGS="-F${PYTHON_FDIR} -framework Python"

         SAVED_CPPFLAGS="$CPPFLAGS"
         SAVED_LDFLAGS="$LDFLAGS"

         CPPFLAGS="$PYTHON_CPPFLAGS $CPPFLAGS"
         LDFLAGS="$PYTHON_LDFLAGS $LDFLAGS"
   
         AC_CHECK_HEADER([Python.h],[],
                         [AC_MSG_NOTICE([Couldn't find Python.h. Building without Python.])
                          have_python_devel=no])

         echo $ECHO_N "checking if we can link against the Python framework... $ECHO_C"
         AC_LINK_IFELSE([int main(int argc, char **argv) { return 0; }],
                        [echo yes],
                        [echo no
                         AC_MSG_NOTICE([Building without Python.])
                         have_python_devel=no])

         CPPFLAGS="$SAVED_CPPFLAGS"
         LDFLAGS="$SAVED_LDFLAGS"

     else 
         # This just checks for a standard GNU install with Python.h and libpython
         PYTHON_LIBNAME="python${PYTHON_VERSION}"

         PYTHON_CPPFLAGS="-I$pyhome/include/python${PYTHON_VERSION}"
         PYTHON_LDFLAGS="-L$pyhome/lib -l${PYTHON_LIBNAME}"
         PYTHON_RPATH="-R $pyhome/lib"

         SAVED_CPPFLAGS="$CPPFLAGS"
         SAVED_LDFLAGS="$LDFLAGS"
         
         CPPFLAGS="$PYTHON_CPPFLAGS $CPPFLAGS"
         LDFLAGS="$PYTHON_LDFLAGS $LDFLAGS"
         
         AC_CHECK_HEADER([Python.h],[],
                         [AC_MSG_NOTICE([Couldn't find Python.h.])
                          have_python_devel=no])
         
         LX_SAFE_CHECK_LIB([${PYTHON_LIBNAME}],
                           [_PyObject_New],[],
                           [AC_MSG_NOTICE([Couldn't find lib${PYTHON_LIBNAME}.])
                            have_python_devel=no])
         
         CPPFLAGS="$SAVED_CPPFLAGS"
         LDFLAGS="$SAVED_LDFLAGS"
     fi

     AC_SUBST(PYTHON_CPPFLAGS)
     AC_SUBST(PYTHON_LDFLAGS)
     AC_SUBST(PYTHON_RPATH)

     if [[ "x$have_python_devel" != xno ]] ; then
         have_python_devel=yes
     fi
 fi
])


