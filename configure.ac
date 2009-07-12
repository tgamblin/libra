#############################################################
# This is the main autoconf file for building AMPL.  
#############################################################
AC_INIT(libwavelet/wt_direct.C)
AC_CONFIG_AUX_DIR(config)

AM_INIT_AUTOMAKE(libra,0.9)
AM_CONFIG_HEADER(config.h)
AX_PREFIX_CONFIG_H([libra-config.h])

# Programs we need
AC_PROG_CXX
AC_PROG_INSTALL

# now that we've specified deps, declare libtool
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

# Fortran binding for MPI_Init
AC_FC_FUNC(PMPI_Init, PMPI_INIT)
AC_SUBST(PMPI_INIT)

# Test whether we're on BlueGene/L and define things in config.h accordingly
AC_LANG_PUSH([C++])
AC_DETECT_BLUEGENE
AC_LANG_POP

# Distributed Sampling Support
AC_SPRNG
if [[ "x$have_sprng" = xyes ]]; then
   AC_DEFINE([HAVE_SPRNG], [1], [Defined if we have the SPRNG library.])
fi


# We need the boost matrix class to build this library.
AX_BOOST_BASE

# Check for MPI.  If we don't have it, don't build parallel
# Assumes that you're using an MPI compiler (mpicxx, etc) that handles libs.
# This will NOT add MPI libraries to the build.
CC=${CXX}          # Use CXX instead of CC for tests
CPP="${CXX} -E"    # Use CXX instead of defaupt CPP for tests
AC_CHECK_HEADER([mpi.h],
    [AC_DEFINE([HAVE_MPI], [1], [Define to 1 if you have <mpi.h>.])
     have_mpi=yes],
    [AC_MSG_NOTICE([Couldn't find mpi.h. Building without MPI.])
     have_mpi=no])
AM_CONDITIONAL([HAVE_MPI], [test "x$have_mpi" == xyes])

# Check for Linux timers.
AC_CHECK_LIB(rt, clock_gettime)
AC_CHECK_FUNCS(clock_gettime gettimeofday)

AC_OPTIONS_VTK
AC_PATH_VTK([5.0.0],
	[AC_MSG_NOTICE([Found VTK Library.])
	 have_vtk=yes],
	[AC_MSG_NOTICE([Couldn't find VTK library.])
	 have_vtk=no]
)

# Would like to use gmake for the build.
# AC_PATH_PROGS(GMAKE,[gmake gnumake],[AC_MSG_ERROR([Build requires gmake.])])

# This tests for a python installation and finds the python binary, the latest python version,
# the Python.h header, and the python library.
AC_PYTHON_DEVEL
AC_PYTHON_MODULE(PyQt4)
AC_PYTHON_MODULE(numpy)

AC_PROG_SWIG(1.3.30)
if (${SWIG} -version &> /dev/null); then
   have_swig=yes
   SWIG_ENABLE_CXX
   SWIG_PYTHON
else
   have_swig=no
   AC_MSG_NOTICE([Couldn't find SWIG.  Will not be able to regenerate pyeffort wrappings.])
fi

if [[ "x$have_python" = xyes -a "x$HAVE_PYMOD_PYQT4" = xyes -a \
      "x$HAVE_PYMOD_NUMPY" = xyes -a "x$have_vtk" = xyes ]]; then
   build_viewer=yes
else
   AC_MSG_NOTICE([Building without viewer.])
   build_viewer=no
fi
AM_CONDITIONAL([BUILD_VIEWER], [ test "x$build_viewer" = xyes ])

# Check for Wisconsin ParaDyn Tools
AC_PARADYN_TOOLS
AM_CONDITIONAL([HAVE_SW], [test "x$have_stackwalk" = xyes])
if [[ "x$have_stackwalk" = xyes ]]; then
   AC_DEFINE([HAVE_SW], [1], [Defined if we have DynStackwalker API.])
fi
AM_CONDITIONAL([HAVE_SYMTAB], [test "x$have_symtabAPI" = xyes])
if [[ "x$have_symtabAPI" = xyes ]]; then
   AC_DEFINE([HAVE_SYMTAB], [1], [Defined if we have SymtabAPI.])
fi


# Check for PAPI for HW counters
AC_LIB_PAPI
AM_CONDITIONAL([HAVE_PAPI],  [test "x$have_papi" = xyes])


# Use PnMPI if the user provided it explicitly, otherwise use PMPI.
if [[ "x$have_mpi" = xyes -a "x$have_stackwalk" = xyes ]]; then
    AC_PNMPI([AC_MSG_NOTICE([Building PnMPI tool modules.])
              AC_DEFINE([PNMPI_EFFORT], [1], [Define to 1 to build PMPI-only effort tool.])
              have_pnmpi=yes],
             [have_pnmpi=no])

    if [[ "x$have_pnmpi" = xno ]]; then
        AC_MSG_NOTICE([Building PMPI tool module.])
        AC_DEFINE([PMPI_EFFORT], [1], [Define to 1 to build PMPI-only effort tool.])
        pmpi_effort=yes
    else
        pmpi_effort=no
    fi
else
    pmpi_effort=no
    have_pnmpi=no
fi
AM_CONDITIONAL([PMPI_EFFORT],  [test "x$pmpi_effort" = xyes])
AM_CONDITIONAL([PNMPI_EFFORT], [test "x$have_pnmpi" = xyes])


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
echo "  MPI ............................................ $have_mpi"
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
echo "  SWIG ........................................... $have_swig"
echo
echo "Build Options:"
echo "  Libra Viewer ................................... $build_viewer"
echo "  PMPI Effort Library ............................ $pmpi_effort"
echo "  PnMPI Effort Library ........................... $have_pnmpi"
echo "  PMPI Wavelet Bindings .......................... $pmpi_wavelet"
echo "  Sampling (SPRNG) ............................... $have_sprng"
echo "========================================================"
echo

AC_CONFIG_FILES([Makefile \
                 libwavelet/Makefile \
                 callpath/Makefile \
                 effort/Makefile \
                 cluster/Makefile \
                 hcluster/Makefile \
                 viewer/Makefile \
                 tests/Makefile])
AC_OUTPUT
