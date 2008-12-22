AUTOMAKE_OPTIONS = foreign   # Don't require NEWS, README, INSTALL, etc.emacs -
ACLOCAL_AMFLAGS = -I m4

if BUILD_VIEWER
VIEWER_DIRS=hcluster viewer
endif

# Build-time configuration of libraries.
include_HEADERS=wavelet-config.h
DISTCLEANFILES = wavelet-config.h _configs.sed

dist_noinst_SCRIPTS = \
	do_configure_atlas \
	do_configure_bgl \
	do_configure_bgl_fen

EXTRA_DIST = LICENSE README

SUBDIRS=libwavelet callpath effort $(VIEWER_DIRS) tests

DISTCHECK_CONFIGURE_FLAGS = \
		CXX=mpiicpc \
    CXXFLAGS='-g -O3 -fno-omit-frame-pointer -DMPICH_IGNORE_CXX_SEEK -DVTK_EXCLUDE_STRSTREAM_HEADERS' \
    --with-paradyn=/usr/local/tools/stackwalker-0.6.1 \
    --with-dwarf=/usr/local/tools/libdwarf-2008.10.13 \
    --with-papi=/usr/local/tools/papi \
    --with-python=/usr/gapps/python/chaos_4_x86_64_ib/opt-2.5 \
    --with-python-version=2.5 \
    --with-vtk=/usr/gapps/python/chaos_4_x86_64_ib/opt-2.5 \
    --disable-static
