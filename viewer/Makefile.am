# We've got two libraries wrapped by different tools.
# One is the SWIG-wrapped effort interface.  The other is special
# vtk-wrapped library.  VTK requires its own special python wrappings.
python_LTLIBRARIES=libvtkeffort.la _pyeffort.la

dist_noinst_SCRIPTS = wrap_vtk.sh

# SWIG-generated files
EXTRA_DIST = pyeffort.i pyeffort.py

###########################################################################
# vtkeffort module is wrapped using VTK's wrapper generators.
# Init function is also generated automatically by VTK's generator.
###########################################################################
VTKEFFORT_H_FILES = \
	vtkEffortData.h \
	vtkEffortSource.h \
	vtkZValueScalars.h

VTKEFFORT_CXX_SOURCES = \
	vtkEffortData.C \
	vtkEffortSource.C \
	vtkZValueScalars.C

VTKEFFORT_PYTHON_SOURCES = \
	libvtkeffortPythonInit.C \
	vtkEffortDataPython.C \
	vtkEffortSourcePython.C \
	vtkZValueScalarsPython.C

$(VTKEFFORT_PYTHON_SOURCES): $(VTKEFFORT_H_FILES)
	$(SHELL) $(srcdir)/wrap_vtk.sh $(VTK_WRAP_PYTHON) $(VTK_WRAP_PYTHON_INIT) $(srcdir) $(VTKEFFORT_PYTHON_SOURCES)

libvtkeffort_la_SOURCES = \
	EffortData.C \
	summary.C \
	wavelet_ssim.C \
	$(VTKEFFORT_CXX_SOURCES) \
	$(VTKEFFORT_PYTHON_SOURCES)

libvtkeffort_la_LIBADD = \
										../effort/libeffort.la \
										../callpath/libcallpath.la \
										../libwavelet/libwavelet.la \
										-lvtkCommonPythonD \
								    -lvtkGraphicsPythonD \
										-lvtkFilteringPythonD \
                    $(VTK_LIBS) \
                    $(PYTHON_LIBS)
libvtkeffort_la_LDFLAGS = -module -avoid-version $(PYTHON_LDFLAGS)

###########################################################################
# pyeffort module needs more things exposed, so we use SWIG to generate
# wrappers for this.  Module naming convention follows SWIG's guidelines.
# Can still "import pyeffort" from python due to the associated pyeffort.py
# file that is generated and installed.
###########################################################################
_pyeffort_la_SOURCES = pyeffort_wrap.C
pyeffort_wrap.C: pyeffort.i
	$(SWIG) $(SWIG_PYTHON_OPT) -I$(top_srcdir)/src -o $@ $<

_pyeffort_la_LIBADD =   libvtkeffort.la \
												../effort/libeffort.la \
						    				../callpath/libcallpath.la \
								    		../libwavelet/libwavelet.la \
                        $(VTK_LIBS) \
										    -lvtkCommonPythonD \
										    -lvtkGraphicsPythonD \
                        $(PYTHON_LIBS)
_pyeffort_la_LDFLAGS = -module -avoid-version $(PYTHON_LDFLAGS)
pyeffort.py: _pyeffort.la

include_HEADERS = \
	EffortData.h \
	summary.h \
	wavelet_ssim.h \
	$(VTKEFFORT_H_FILES)

#
# Python scripts that should go in /bin
#
dist_bin_SCRIPTS = libra libra-view

#
# Python modules to be byte-compiled and put in /lib/pythonX.X/site-packages.
#
python_PYTHON = \
	dendrogram.py \
	effort.py \
  effort_tree.py \
  surface.py \
  viewer.py \
  vtkutils.py \
	pyeffort.py \
	vtkeffort.py \
	icons.py \
	source.py 

#
# Set up a directory for icons and install all the graphics there.
#
icondir=${pkgdatadir}/icons
dist_icon_DATA = \
	icons/file-new.png \
	icons/multiscale.png \
	icons/folder.png \
	icons/up_triangle.gif \
	icons/down_triangle.gif \
	icons/dendrogram.gif \
	icons/grid.png \
	icons/close.png \
	icons/pushpin.png \
	icons/crosshair.png \
	icons/expand.gif \
	icons/libra_icon64.png \
	icons/libra_icon128.png \
	icons/libra_logo64.png \
	icons/libra_logo128.png \
	icons/libra_logo256.png \
	icons/terminal.png


noinst_PROGRAMS=simtest
simtest_SOURCES=simtest.C
simtest_LDADD=libvtkeffort.la ../libwavelet/libwavelet.la 

INCLUDES=$(BOOST_CPPFLAGS) $(VTK_CPPFLAGS) -I$(top_srcdir)/libwavelet \
-I$(top_srcdir)/callpath -I$(top_srcdir)/effort $(PYTHON_CPPFLAGS)

CLEANFILES=$(VTKEFFORT_PYTHON_SOURCES) libvtkpython_init.txt


