#
# Wavelet library and associated sources.
#
lib_LTLIBRARIES = libwavelet.la
libwavelet_la_SOURCES = \
	cdf97.C \
	wt_2d.C \
	wt_lift.C \
	wt_direct.C \
	wt_1d_direct.C \
	wt_utils.C \
	io_utils.C \
	matrix_utils.C \
	filter_bank.C \
	ezw.C \
	ezw_encoder.C \
	ezw_decoder.C \
	obitstream.C \
	ibitstream.C \
	buffered_obitstream.C \
	buffered_ibitstream.C \
	vector_obitstream.C \
	vector_ibitstream.C \
	ac_obitstream.C \
	ac_ibitstream.C \
	arithmetic_codec.C \
	byte_budget_exception.C \
	timing.C \
	Timer.C \
	rle.C \
	huffman.C \
	string_utils.C
libwavelet_la_LDFLAGS=-avoid-version

#
# Parallel sources for wavelet library, if we have MPI.
#
if HAVE_MPI
libwavelet_la_SOURCES += \
	wt_parallel.C \
	par_ezw_encoder.C
endif

#
# Headers for all the library classes
#
include_HEADERS = \
	ac_obitstream.h \
	ac_ibitstream.h \
	buffered_obitstream.h \
	buffered_ibitstream.h \
	byte_budget_exception.h \
	cdf97.h \
	ezw.h \
	ezw_encoder.h \
	ezw_decoder.h \
	filter_bank.h \
	ibitstream.h \
	io_utils.h \
	matrix_utils.h \
	obitstream.h \
	stl_utils.h \
	timing.h \
	Timer.h \
	vector_ibitstream.h \
	vector_obitstream.h \
	wavelet.h \
	wt_2d.h \
	wt_direct.h \
	wt_1d_direct.h \
	wt_lift.h \
	string_utils.h

dist_noinst_HEADERS = \
  arithmetic_codec.h \
	mpi_profile.h \
	mpi_utils.h \
	huffman.h \
	rle.h \
	wt_utils.h


#
# Parallel wt headers, only if we found MPI.
#
if HAVE_MPI
include_HEADERS += wt_parallel.h par_ezw_encoder.h
endif

#
# These headers are not installed by default.
#
devel_headers = \
	wt_utils.h \
	timing.h \
	rle.h \
	huffman.h


INCLUDES=$(BOOST_CPPFLAGS)
