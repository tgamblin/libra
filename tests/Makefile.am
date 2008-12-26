noinst_PROGRAMS = compress_matfile  vary_passes \
							    insert_bits_test ezwtest seqtest vltest

TESTS = seqtest ezwtest insert_bits_test vltest

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

papicheck_SOURCES = papicheck.C
papicheck_CPPFLAGS=$(PAPI_CPPFLAGS)
papicheck_LDADD=$(PAPI_LDFLAGS)

swcheck_SOURCES = swcheck.C
swcheck_LDADD = $(SW_LDFLAGS)
swcheck_CPPFLAGS = -I../callpath $(SW_CPPFLAGS)

partest_SOURCES = partest.C
parezwtest_SOURCES = parezwtest.C
parspeedbench_SOURCES = parspeedbench.C

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

LDADD = ../callpath/libcallpath.la ../libwavelet/libwavelet.la
INCLUDES = $(BOOST_CPPFLAGS) -I$(top_srcdir)/libwavelet -I$(top_srcdir)/effort
