
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
	sampling.h \
	ltqnorm.h

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
											 effort_dataset.C \
											 s3d_topology.C

if HAVE_MPI
# move this to runtime?
libeffort_la_SOURCES += parallel_compressor.C \
												parallel_decompressor.C \
												synchronize_keys.C

if HAVE_SPRNG
libeffort_la_SOURCES += sampling.C ltqnorm.C
endif
endif

libeffort_la_LDFLAGS = -avoid-version
libeffort_la_LIBADD = \
	../callpath/libcallpath.la \
	../libwavelet/libwavelet.la \
	$(PAPI_LDFLAGS) \
	$(SPRNG_LDFLAGS)

#
# Effort runtime library and wrapper libraries
#
if PMPI_EFFORT
lib_LTLIBRARIES += libpmpi-effort.la libtiming.la libpcontrol-counter.la libeffort-runtime.la libcomm-effort.la 

libpcontrol_counter_la_SOURCES = pcontrol_counter.C

libeffort_runtime_la_SOURCES = effort_module.C \
															 env_config.C \
															 sampling.C \
															 base_wrapper.C
libeffort_runtime_la_LIBADD = libeffort.la
libeffort_runtime_la_LDFLAGS = -avoid-version 

libpmpi_effort_la_SOURCES = effort_wrapper.C
libpmpi_effort_la_LIBADD = libeffort-runtime.la
libpmpi_effort_la_LDFLAGS = -avoid-version 

libcomm_effort_la_SOURCES = comm_wrapper.C
libcomm_effort_la_LIBADD = libeffort-runtime.la
libcomm_effort_la_LDFLAGS = -avoid-version 

libtiming_la_SOURCES = timing_module.C
libtiming_la_LDFLAGS = -avoid-version
libtiming_la_LIBADD = ../libwavelet/libwavelet.la
endif


#
# Effort utility programs
#
bin_PROGRAMS = ef nrmse s3d-topo-test sam
if HAVE_MPI
bin_PROGRAMS += tuner bin_test dataset_test sample-test approx-timer 
endif

dist_bin_SCRIPTS= libra-build-viewer-data libra-reconstruct-exact

approx_timer_SOURCES = approx_timer.C
approx_timer_LDADD=libeffort.la ../cluster/libcluster.la

sample_test_SOURCES = sample_test.C
sample_test_LDADD=libeffort.la ../cluster/libcluster.la

dataset_test_SOURCES = dataset_test.C
dataset_test_LDADD=libeffort.la ../cluster/libcluster.la

ef_SOURCES=ef.C
ef_LDADD=libeffort.la $(SYMTAB_LDFLAGS)

sam_SOURCES=sam.C
sam_LDADD=libeffort.la $(SYMTAB_LDFLAGS)

nrmse_SOURCES=nrmse.C 
tuner_SOURCES=tuner.C
bin_test_SOURCES=bin_test.C
s3d_topo_test_SOURCES=s3d_topo_test.C

CLEANFILES = effort_wrapper.C comm_wrapper.C
EXTRA_DIST = effort_wrapper.w comm_wrapper.w
dist_noinst_SCRIPTS = wrap.py

base_wrapper.C: base_wrapper.w
	$(PYTHON) $(srcdir)/wrap.py -fg -i $(PMPI_INIT) $< -o $@

effort_wrapper.C: effort_wrapper.w
	$(PYTHON) $(srcdir)/wrap.py -fg -i $(PMPI_INIT) $< -o $@

comm_wrapper.C: comm_wrapper.w
	$(PYTHON) $(srcdir)/wrap.py -fg -i $(PMPI_INIT) $< -o $@

waitall_wrapper.C: waitall_wrapper.w
	$(PYTHON) $(srcdir)/wrap.py -fg -i $(PMPI_INIT) $< -o $@


LDADD   =  libeffort.la
INCLUDES = -I$(top_srcdir)/libwavelet \
           -I$(top_srcdir)/callpath \
           -I$(top_srcdir)/cluster \
					 $(SPRNG_CPPFLAGS) \
           $(PNMPI_CPPFLAGS) \
           $(BOOST_CPPFLAGS) \
           $(PARADYN_CPPFLAGS) \
           $(PAPI_CPPFLAGS) \
           $(SW_CPPFLAGS) \
           $(AMPL_CPPFLAGS)







#
# Build a PnMPI module and install in PnMPI's module directory.
# NOTE: below is currently unmaintained and unsupported.
#
if PNMPI_EFFORT
pnmpi_LTLIBRARIES = effort.la timing.la

timing_la_SOURCES = timing_module.C
timing_la_LDFLAGS = -avoid-version -module
timing_la_LIBADD = ../libwavelet/libwavelet.la

effort_la_SOURCES = $(PMPI_EFFORT_SRCS) effort_wrapper.C
effort_la_LIBADD = ../callpath/libcallpath.la ../libwavelet/libwavelet.la $(PAPI_LDFLAGS) $(SPRNG_LDFLAGS)
effort_la_LDFLAGS = -avoid-version -module

# Installs library using PnMPI patcher
install-exec-hook:
	$(PNMPI_PATCH) .libs/effort.so .libs/effort-patched.so
	chmod u+x .libs/effort-patched.so
	mv -f .libs/effort-patched.so .libs/effort.so
endif

