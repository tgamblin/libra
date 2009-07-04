
include_HEADERS = effort_api.h
dist_noinst_HEADERS = \
	effort_data.h \
	effort_key.h \
	effort_module.h \
	effort_params.h \
	effort_record.h \
	parallel_compressor.h \
	parallel_decompressor.h \
	env_config.h \
	string_utils.h \
	synchronize_keys.h \
	Metric.h \
  effort_dataset.h \
	FrameDB.h \
	FrameInfo.h

lib_LTLIBRARIES = libeffort.la

#
# This is used by 
#
libeffort_la_SOURCES = effort_key.C \
						 		       effort_record.C \
                       effort_data.C \
                       effort_params.C \
											 string_utils.C \
											 Metric.C \
											 FrameDB.C \
											 FrameInfo.C \
											 effort_dataset.C

if HAVE_MPI
libeffort_la_SOURCES += parallel_compressor.C \
												parallel_decompressor.C \
												synchronize_keys.C
endif
libeffort_la_LDFLAGS = -avoid-version

#
# Sources for both PMPI and PnMPI library
#
PMPI_EFFORT_SRCS=effort_module.C \
								 env_config.C

#
# Effort utility programs
#
bin_PROGRAMS = ef nrmse 
if HAVE_MPI
bin_PROGRAMS += tuner bin_test dataset_test sample-test approx-timer
endif

dist_bin_SCRIPTS= libra-build-viewer-data libra-reconstruct-exact

approx_timer_SOURCES = approx_timer.C
approx_timer_LDADD=libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la \
                   ../cluster/libcluster.la $(SYMTAB_LDFLAGS) 

sample_test_SOURCES = sample_test.C
sample_test_LDADD=libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la \
                   ../cluster/libcluster.la $(SYMTAB_LDFLAGS) 

dataset_test_SOURCES = dataset_test.C
dataset_test_LDADD=libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la \
                   $(SYMTAB_LDFLAGS) 

ef_SOURCES=ef.C
ef_LDADD=libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la $(SYMTAB_LDFLAGS) 

nrmse_SOURCES=nrmse.C 
nrmse_LDADD=libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la

tuner_SOURCES=tuner.C
tuner_LDADD=libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la

bin_test_SOURCES=bin_test.C
bin_test_LDADD=libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la

#
# Build a regular PMPI module and install in $(prefix)/lib
#
if PMPI_EFFORT
lib_LTLIBRARIES += libpmpi-effort.la libtiming.la libpcontrol-counter.la libmanual-effort.la libcomm-effort.la

libpcontrol_counter_la_SOURCES = pcontrol_counter.C

libpmpi_effort_la_SOURCES = $(PMPI_EFFORT_SRCS) effort_wrapper.C
libpmpi_effort_la_LIBADD = libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la $(SW_LDFLAGS) $(PAPI_LDFLAGS)
libpmpi_effort_la_LDFLAGS = -avoid-version 

libcomm_effort_la_SOURCES = $(PMPI_EFFORT_SRCS) comm_wrapper.C
libcomm_effort_la_LIBADD = libeffort.la ../callpath/libcallpath.la ../libwavelet/libwavelet.la $(SW_LDFLAGS) $(PAPI_LDFLAGS)
libcomm_effort_la_LDFLAGS = -avoid-version 

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

CLEANFILES = effort_wrapper.C comm_wrapper.C
EXTRA_DIST = effort_wrapper.w comm_wrapper.w
dist_noinst_SCRIPTS = wrap.py

effort_wrapper.C: effort_wrapper.w
	$(PYTHON) $(srcdir)/wrap.py -fg -i $(PMPI_INIT) $< -o $@

comm_wrapper.C: comm_wrapper.w
	$(PYTHON) $(srcdir)/wrap.py -fg -i $(PMPI_INIT) $< -o $@


INCLUDES= $(PNMPI_CPPFLAGS) $(BOOST_CPPFLAGS) -I$(top_srcdir)/libwavelet \
 -I$(top_srcdir)/callpath -I$(top_srcdir)/cluster $(PARADYN_CPPFLAGS) $(PAPI_CPPFLAGS) $(SW_CPPFLAGS)
