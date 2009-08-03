#
# The Callpath class itself is a separate library b/c it's used by 
# tools and other libraries (like libeffort).
#

if HAVE_SW
SW_ONLY_SRCS = CallpathRuntime.C 
endif

lib_LTLIBRARIES = libcallpath.la
libcallpath_la_SOURCES = \
	Callpath.C \
	FrameId.C \
	ModuleId.C \
	FrameInfo.C \
	Translator.C \
	$(SW_ONLY_SRCS)
libcallpath_la_LIBADD = $(SW_LDFLAGS)
libcallpath_la_LDFLAGS = -avoid-version 
include_HEADERS = \
	FrameId.h \
	Callpath.h \
	CallpathRuntime.h \
  UniqueId.h \
	ModuleId.h \
	FrameInfo.h \
	Translator.h

INCLUDES = $(SW_CPPFLAGS) -I$(top_srcdir)/libwavelet $(BOOST_CPPFLAGS) -I$(top_srcdir)

#
# Below section builds a PnMPI module to trace callpaths using
# the Callpath class.
#
if PNMPI_EFFORT
pnmpi_LTLIBRARIES = callpath.la
nodist_callpath_la_SOURCES = callpath_wrapper.c
callpath_la_SOURCES = callpath_module.C
callpath_la_LDFLAGS = -module -avoid-version
callpath_la_LIBADD = libcallpath.la $(SW_LDFLAGS)
include_HEADERS += callpath_module.h

install-exec-hook:
	$(PNMPI_PATCH) .libs/callpath.so .libs/callpath-patched.so
	chmod u+x .libs/callpath-patched.so
	mv -f .libs/callpath-patched.so .libs/callpath.so

callpath_wrapper.c: callpath_wrapper.w
	$(PNMPI_WRAPPERGEN) -w $< -o $@

endif
