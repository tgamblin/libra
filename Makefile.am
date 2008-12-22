AUTOMAKE_OPTIONS = foreign   # Don't require NEWS, README, INSTALL, etc.emacs -
ACLOCAL_AMFLAGS = -I m4

if BUILD_VIEWER
VIEWER_DIRS=hcluster viewer
endif

# Build-time configuration of libraries.
include_HEADERS=libra-config.h
DISTCLEANFILES = libra-config.h _configs.sed

dist_noinst_SCRIPTS = \
	do_configure_atlas \
	do_configure_bgl \
	do_configure_bgl_fen

EXTRA_DIST = LICENSE README

SUBDIRS=libwavelet callpath effort $(VIEWER_DIRS) tests

