#############################################################
# This is the main autoconf file for building Libra.
#############################################################
AC_INIT
AC_CONFIG_SRCDIR([libwavelet/wt_direct.C])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE(libra,1.0)
AC_CONFIG_HEADERS([config.h])
AX_PREFIX_CONFIG_H([libra-config.h])

# Programs we need
AC_PROG_CXX
AC_PROG_FC
AC_PROG_INSTALL

# now that we've specified deps, declare libtool
LT_INIT
AC_SUBST(LIBTOOL_DEPS)

# Find a fortran binding for MPI_Init
AC_FC_FUNC(PMPI_Init, PMPI_INIT)
AC_ARG_WITH([pmpi-init], 
  AS_HELP_STRING([--with-pmpi-init], 
                 [Manually specify binding for pmpi_init function.  Must be one of PMPI_INIT, pmpi_init, pmpi_init_, or pmpi_init__]),
  [if [[ "x$withval" = xpmpi_init -o "x$withval" = xPMPI_INIT -o "x$withval" = xpmpi_init_ -o "x$withval" = xpmpi_init__ ]]; then
      PMPI_INIT="$withval"
   else
      AC_MSG_ERROR([--with-pmpi-init arg must be one of PMPI_INIT, pmpi_init, pmpi_init_, or pmpi_init__])
   fi]
)
AC_SUBST(PMPI_INIT)


#
# Test for various C++ libraries
#
AC_LANG_PUSH([C++])
    # Test whether we're on BlueGene/L and define things in config.h accordingly
    LX_DETECT_BLUEGENE

    # Make sure we can compile MPI programs in C++
    LX_FIND_MPI
    AM_CONDITIONAL([HAVE_MPI], [test "x$have_CXX_mpi" == xyes])

    # We need the boost matrix class to build this library.
    AX_BOOST_BASE

    # Need VTK to build the GUI.
    AC_OPTIONS_VTK
    AC_PATH_VTK([5.0.0],
                [AC_MSG_NOTICE([Found VTK Library.]);         have_vtk=yes],
                [AC_MSG_NOTICE([Couldn't find VTK library.]); have_vtk=no])

    # Check for Wisconsin ParaDyn Tools (this includes checking for dependencies)
    LX_PARADYN_TOOLS
    AM_CONDITIONAL([HAVE_SW], [test "x$have_stackwalk" = xyes])
    AM_CONDITIONAL([HAVE_SYMTAB], [test "x$have_symtabAPI" = xyes])
AC_LANG_POP

# Distributed Sampling Support -- SPRNG library
LX_SPRNG
if [[ "x$have_sprng" = xyes ]]; then
   AC_DEFINE([HAVE_SPRNG], [1], [Defined if we have the SPRNG library.])
fi
AM_CONDITIONAL([HAVE_SPRNG], [test "x$have_sprng" == xyes])

# Currently unsupported -- need to add detection of Muster library.
AM_CONDITIONAL([HAVE_MUSTER], [test "x$have_muster" == xyes])


# Check for Linux timers.
AC_CHECK_LIB(rt, clock_gettime)
AC_CHECK_FUNCS(clock_gettime gettimeofday)


# This tests for a python installation and finds the python binary, the latest python version,
# the Python.h header, and the python library.
LX_PYTHON_DEVEL
AX_PYTHON_MODULE(PyQt4)
AX_PYTHON_MODULE(numpy)
AX_PYTHON_MODULE(hcluster)

AX_PKG_SWIG(1.3.30, 
            [ have_swig=yes ],
            [ have_swig=no
              AC_MSG_ERROR([Couldn't find SWIG >= 1.3.30.])])
AX_SWIG_ENABLE_CXX
AX_SWIG_PYTHON

if [[ "x$have_python"         = xyes \
   -a "x$HAVE_PYMOD_PYQT4"    = xyes \
   -a "x$HAVE_PYMOD_NUMPY"    = xyes \
   -a "x$HAVE_PYMOD_HCLUSTER" = xyes \
    -a "x$have_vtk" = xyes ]]; then
   build_viewer=yes
else
   AC_MSG_NOTICE([Building without viewer.])
   build_viewer=no
fi
AM_CONDITIONAL([BUILD_VIEWER], [ test "x$build_viewer" = xyes ])


# Check for PAPI for HW counters
LX_LIB_PAPI
AM_CONDITIONAL([HAVE_PAPI],  [test "x$have_papi" = xyes])

# Use PnMPI if the user provided it explicitly, otherwise use PMPI.
if [[ "x$have_CXX_mpi" = xyes -a "x$have_stackwalk" = xyes ]]; then
    AC_MSG_NOTICE([Building PMPI tool module.])
    AC_DEFINE([PMPI_EFFORT], [1], [Define to 1 to build PMPI-only effort tool.])
    pmpi_effort=yes
else
    pmpi_effort=no
fi
AM_CONDITIONAL([PMPI_EFFORT],  [test "x$pmpi_effort" = xyes])

# Check for PnMPI tool framework.
LX_PNMPI([AC_MSG_NOTICE([Building PnMPI tool modules.])],
         [AC_MSG_NOTICE([Building without PnMPI tool modules.])])
AM_CONDITIONAL([HAVE_PNMPI], [test "x$have_pnmpi" = xyes])

# If this is provided, use PMPI bindings instead of MPI bindings.  
AC_ARG_WITH([mpi-wavelet], 
  AS_HELP_STRING([--with-mpi-wavelet], 
                 [Optionally build the wavelet library with MPI bindings instead of PMPI bindings. Defaults to use PMPI bindings.]),
  [if [[ "x$withval" = xyes ]]; then
      pmpi_wavelet=no
   else
      pmpi_wavelet=yes
   fi],
  [pmpi_wavelet=yes]
)
if [[ "x$pmpi_wavelet" = xyes ]]; then
    AC_DEFINE([USE_PMPI], [1], [Define to 1 to use PMPI bindings for the parallel transform.])
    AC_MSG_NOTICE([Building parallel transform with PMPI bindings.])
else
    AC_MSG_NOTICE([Building parallel transform with MPI bindings.])
fi

echo
echo "========================================================"
echo "==         Libra: final build configuration           =="
echo "========================================================"
echo "External Library Support:"
echo "  MPI ............................................ $have_CXX_mpi"
echo "  VTK ............................................ $have_vtk"
echo "  PAPI ........................................... $have_papi"
echo "  ParaDyn - Common ............................... $have_common"
echo "  ParaDyn - SymtabAPI ............................ $have_symtabAPI"
echo "  ParaDyn - Stackwalker .......................... $have_stackwalk"
echo
echo "Python Support:"
echo "  Python Interpreter ............................. $have_python"
echo "  Python Development Headers/Libs ................ $have_python_devel"
echo "  PyQt4 Module ................................... $HAVE_PYMOD_PYQT4"
echo "  Numpy Module ................................... $HAVE_PYMOD_NUMPY"
echo "  SciPy Hcluster ................................. $HAVE_PYMOD_HCLUSTER"
echo "  SWIG ........................................... $have_swig"
echo
echo "Build Options:"
echo "  Libra Viewer ................................... $build_viewer"
echo "  PMPI Effort Library ............................ $pmpi_effort"
echo "  PnMPI Modules .................................. $have_pnmpi"
echo "  PMPI Wavelet Bindings .......................... $pmpi_wavelet"
echo "  Sampling (SPRNG) ............................... $have_sprng"
echo "========================================================"
echo

AC_CONFIG_FILES([Makefile \
                 libwavelet/Makefile \
                 callpath/Makefile \
                 effort/Makefile \
                 viewer/Makefile \
                 tests/Makefile])
AC_OUTPUT
