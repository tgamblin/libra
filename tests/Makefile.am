noinst_PROGRAMS = compress_matfile  vary_passes \
							    insert_bits_test ezwtest seqtest vltest \
								  generictest

TESTS = seqtest ezwtest insert_bits_test vltest

EXTRA_DIST = bunny.dat

if HAVE_MPI
noinst_PROGRAMS += partest parezwtest parspeedbench
TESTS += parezwtest partest 
endif

if PMPI_EFFORT
noinst_PROGRAMS += bunny 
endif

if HAVE_SYMTAB
if HAVE_SW
noinst_PROGRAMS += swcheck
endif
endif

if HAVE_PAPI
noinst_PROGRAMS += papicheck
endif

compress_matfile_SOURCES = compress_matfile.C

seqtest_SOURCES = seqtest.C
ezwtest_SOURCES = ezwtest.C
insert_bits_test_SOURCES = insert_bits_test.C
vary_passes_SOURCES = vary_passes.C
vltest_SOURCES = vltest.C
generictest_SOURCES = generictest.C

papicheck_SOURCES = papicheck.C
papicheck_CPPFLAGS = $(PAPI_CPPFLAGS)
papicheck_LDADD = $(PAPI_LDFLAGS) $(PAPI_RPATH)

swcheck_SOURCES = swcheck.C
swcheck_CPPFLAGS = -I../callpath $(SW_CPPFLAGS)
swcheck_LDADD =  ../callpath/libcallpath.la  ../libwavelet/libwavelet.la

partest_SOURCES = partest.C
partest_LDADD = ../libwavelet/libwavelet.la $(MPI_CXXLDFLAGS)

parezwtest_SOURCES = parezwtest.C
parezwtest_LDADD = ../libwavelet/libwavelet.la $(MPI_CXXLDFLAGS)

parspeedbench_SOURCES = parspeedbench.C
parspeedbench_LDADD = ../libwavelet/libwavelet.la $(MPI_CXXLDFLAGS)

bunny_SOURCES = bunny.C
bunny_CPPFLAGS = -DSRC_DIR="\"$(srcdir)\""
bunny_LDADD = ../effort/libmanual-effort.la


#
# This will pre-relink all the lt binaries.  This is a workaround for clusters where 
# the compiler isn't runnable on the compute nodes (e.g. in the case of icc, compute
# nodes might not be able to access a license server).
#
pre: all
	for prog in $(noinst_PROGRAMS); do if [ -e "$$prog" ]; then $(top_srcdir)/prerelink $$prog; fi; done

delitter:
	rm -f *.out

clean-local: delitter

LDADD = ../libwavelet/libwavelet.la
INCLUDES = \
	$(MPI_CXXFLAGS) \
	$(BOOST_CPPFLAGS) \
	-I$(top_srcdir)/libwavelet \
	-I$(top_srcdir)/effort \
	-I$(top_srcdir)/callpath

