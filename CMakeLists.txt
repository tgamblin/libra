###############################################################
# Libra
# by Todd Gamblin
# tgamblin@llnl.gov
###############################################################
project(libra)
cmake_minimum_required(VERSION 2.8)

# Version information comes from here.  Incrememnt these when new stuff happens.
set(LIBRA_MAJOR_VERSION 1)
set(LIBRA_MINOR_VERSION 0)
set(LIBRA_PATCH_VERSION 0)
set(LIBRA_VERSION "${LIBRA_MAJOR_VERSION}.${LIBRA_MINOR_VERSION}.${LIBRA_PATCH_VERSION}")

# Build shared libraries by default.
get_property(DEFAULT_SHARED GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
option(BUILD_SHARED_LIBS "Build shared libraries?" ${DEFAULT_SHARED})

# Whether to build for regular MPI apps or with PMPI bindings (for tools).
option(LIBRA_USE_PMPI "Build with PMPI bindings?" FALSE)

# Muster requires boost for matrices and random number generators.
find_package(Boost REQUIRED)

# Find the MPI library and set some definitions
# This line ensures that we skpi C++ headers altogether, avoiding unnecessary symbols in the .o files.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOMPI_SKIP_MPICXX -DMPICH_SKIP_MPICXX")
find_package(MPI QUIET)
if (MPI_FOUND)
  set(LIBRA_HAVE_MPI TRUE)
endif()

# Check for various timing functions, so we can support highest-resolution timers available.
include(CheckFunctionExists)

set(CMAKE_REQUIRED_INCLUDES  time.h sys/time.h)
set(CMAKE_REQUIRED_LIBRARIES rt)
check_function_exists(clock_gettime LIBRA_HAVE_CLOCK_GETTIME)

unset(CMAKE_REQUIRED_LIBRARIES)
check_function_exists(gettimeofday LIBRA_HAVE_GETTIMEOFDAY)

# Find viewer stuff here.
find_package(PythonInterp)
find_package(PythonLibs)
find_package(SWIG)
find_package(Qt4)
find_package(VTK)
# TODO: check python packages (PyQt4, hcluster, scipy, numpy)




# Configure a header file with all the variables we found.
configure_file(
  ${PROJECT_SOURCE_DIR}/libra-config.h.in
  ${PROJECT_BINARY_DIR}/libra-config.h)

install(FILES ${PROJECT_BINARY_DIR}/libra-config.h DESTINATION include)

# Configure subdirectories here, after everything else, so that they 
# inherit variables set in the global scope.
add_subdirectory(libwavelet)
add_subdirectory(callpath)
add_subdirectory(effort)
add_subdirectory(viewer)

