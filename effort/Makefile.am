
include_HEADERS = effort_api.h
dist_noinst_HEADERS = \
	effort_data.h \
	effort_key.h \
	effort_module.h \
	effort_params.h \
	effort_record.h \
	parallel_compressor.h \
	env_config.h \
	string_utils.h \
	synchronize_keys.h \
	Metric.h

lib_LTLIBRARIES = libeffort.la

#
# This is used by 
#
libeffort_la_SOURCES = effort_key.C \
						 		       effort_record.C \
                       effort_data.C \
                       effort_params.C \
											 string_utils.C \
											 Metric.C
libeffort_la_LDFLAGS = -avoid-version

#
# Sources for both PMPI and PnMPI library
#
PMPI_EFFORT_SRCS=effort_module.C \
								 parallel_compressor.C \
						 		 synchronize_keys.C \
								 env_config.C

#
# Effort utility programs
#
if HAVE_SYMTAB
SYMTAB_PROGS=ef
endif

bin_PROGRAMS = $(SYMTAB_PROGS) nrmse 
if HAVE_MPI
bin_PROGRAMS += tuner
endif

dist_bin_SCRIPTS= libra-build-viewer-data libra-reconstruct-exact

ef_SOURCES=ef.C
ef_LDADD=libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la $(SYMTAB_LDFLAGS) 

nrmse_SOURCES=nrmse.C 
nrmse_LDADD=libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la

tuner_SOURCES=tuner.C
tuner_LDADD=libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la

#
# Build a regular PMPI module and install in $(prefix)/lib
#
if PMPI_EFFORT
lib_LTLIBRARIES += libpmpi-effort.la libtiming.la libpcontrol-counter.la libmanual-effort.la

libpcontrol_counter_la_SOURCES = pcontrol_counter.C

libpmpi_effort_la_SOURCES = $(PMPI_EFFORT_SRCS) effort_wrapper.C
libpmpi_effort_la_LIBADD = libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la $(SW_LDFLAGS) $(PAPI_LDFLAGS)
libpmpi_effort_la_LDFLAGS = -avoid-version 

libmanual_effort_la_SOURCES = $(PMPI_EFFORT_SRCS)
libmanual_effort_la_LIBADD = libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la $(SW_LDFLAGS) $(PAPI_LDFLAGS)
libmanual_effort_la_LDFLAGS = -avoid-version 

libtiming_la_SOURCES = timing_module.C
libtiming_la_LDFLAGS = -avoid-version
libtiming_la_LIBADD = ../libwavelet/libwavelet.la
endif

#
# Build a PnMPI module and install in PnMPI's module directory.
#
if PNMPI_EFFORT
pnmpi_LTLIBRARIES = effort.la timing.la

timing_la_SOURCES = timing_module.C
timing_la_LDFLAGS = -avoid-version -module
timing_la_LIBADD = ../libwavelet/libwavelet.la

effort_la_SOURCES = $(PMPI_EFFORT_SRCS) effort_wrapper.C
effort_la_LIBADD = ../callpath/libcallpath.la ../libwavelet/libwavelet.la $(SW_LDFLAGS) $(PAPI_LDFLAGS)
effort_la_LDFLAGS = -avoid-version -module

# Installs library using PnMPI patcher
install-exec-hook:
	$(PNMPI_PATCH) .libs/effort.so .libs/effort-patched.so
	chmod u+x .libs/effort-patched.so
	mv -f .libs/effort-patched.so .libs/effort.so
endif

CLEANFILES=effort_wrapper.C
EXTRA_DIST=effort_wrapper.w
dist_noinst_SCRIPTS = wrap.py
effort_wrapper.C: effort_wrapper.w
	$(PYTHON) $(srcdir)/wrap.py -fg -i $(PMPI_INIT) $< -o $@

INCLUDES= $(PNMPI_CPPFLAGS) $(BOOST_CPPFLAGS) -I$(top_srcdir)/libwavelet \
 -I$(top_srcdir)/callpath $(PARADYN_CPPFLAGS) $(PAPI_CPPFLAGS) $(SW_CPPFLAGS)
