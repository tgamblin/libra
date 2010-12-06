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
#  Adds --with-python to configure and searches for a python-config.  Uses this
#  to get includes, libs, etc. for the python installation.  This should work on 
#  Mac OS X, which comes with python-config now as well.
#
#  If python-config is not found, this just sets the python executable.
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
AC_DEFUN([LX_PYTHON_DEVEL], [
    echo "checking for python..."

    AC_ARG_WITH([python], AS_HELP_STRING([--with-python], [Specify the location of your python interpreter.]),
    [   # Make sure this python is executable.
        if [[ ! -x "$withval" ]]; then
            AC_MSG_ERROR(["$withval" is not a valid Python interpreter.])
            have_python=no
        else
            have_python=yes
        fi
        PYTHON="$withval"
    ],
    [   AC_PATH_PROGS(PYTHON, [python2.7 python2.6 python2.5 python2.4 python])
        if [[ x$PYTHON != x ]]; then
            AC_SUBST(PYTHON)
            have_python=yes
        else
            have_python=no
        fi
    ])

    # Should have pyhome and PYTHON_VERSION set here.  Now test for libs, etc. and executables.
    if [[ "x$have_python" = xyes ]]; then
        echo "found Python interpreter: ${PYTHON}."

        # Make sure the version is set before continuing.
        PYTHON_VERSION=`${PYTHON} -V 2>&1 | sed 's_^[[A-Za-z]]* \([[0-9]]\.[[0-9]]\).*$_\1_'`
        AC_SUBST(PYTHON_VERSION)
        echo "Python version: ${PYTHON_VERSION}."

        # Use python-config to get the rest of the python information
        have_python_devel=no
        PYTHON_CONFIG="${PYTHON}-config"
        if [[ -x "${PYTHON_CONFIG}" ]] ; then
            have_python_devel=yes

            # First set up all the variables that AM_PATH_PYTHON does.
            AC_SUBST(PYTHON_PREFIX)
            PYTHON_PREFIX=$(${PYTHON_CONFIG} --prefix)

            AC_SUBST(PYTHON_EXEC_PREFIX)
            PYTHON_EXEC_PREFIX=$(${PYTHON_CONFIG} --exec-prefix)
     
            AC_SUBST(PYTHON_PLATFORM)
            PYTHON_PLATFORM=`$PYTHON -c "import sys; print sys.platform"`
                              
            AC_SUBST(pythondir)
            pythondir="${prefix}/lib/python${PYTHON_VERSION}/site-packages"
     
            AC_SUBST(pkgpythondir)
            pkgpythondir=${pythondir}/${PACKAGE}
     
            AC_SUBST(pyexecdir)
            pyexecdir=$pythondir
     
            AC_SUBST(pkgpyexecdir)
            pkgpyexecdir=$pkgpythondir

            #
            # Now make sure we can compile and link
            #
            PYTHON_CPPFLAGS=$($PYTHON_CONFIG --includes)
            PYTHON_LDFLAGS="-L${PYTHON_PREFIX}/lib $($PYTHON_CONFIG --ldflags)"

            SAVED_CPPFLAGS="$CPPFLAGS"
            SAVED_LDFLAGS="$LDFLAGS"

            CPPFLAGS="$PYTHON_CPPFLAGS $CPPFLAGS"
            LDFLAGS="$PYTHON_LDFLAGS $LDFLAGS"
   
            AC_CHECK_HEADER([Python.h],[],
                            [AC_MSG_NOTICE([Couldn't find Python.h. Building without Python.])
                             have_python_devel=no])

            echo $ECHO_N "checking if we can link Python library... $ECHO_C"
            AC_LINK_IFELSE([int main(int argc, char **argv) { return 0; }],
                           [echo yes],
                           [echo no
                            AC_MSG_NOTICE([Building without Python.])
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


