set(WAVELET_SOURCES
	cdf97.C 
	wt_1d.C 
	wt_2d.C 
	wt_lift.C 
	wt_direct.C 
	wt_1d_lift.C 
	wt_1d_direct.C 
	wt_utils.C 
	io_utils.C
	matrix_utils.C
	filter_bank.C
	ezw.C
	ezw_encoder.C
	ezw_decoder.C
	obitstream.C
	ibitstream.C
	buffered_obitstream.C
	buffered_ibitstream.C
	vector_obitstream.C
	vector_ibitstream.C
	ac_obitstream.C
	ac_ibitstream.C
	arithmetic_codec.C
	byte_budget_exception.C
	timing.C
	Timer.C
	rle.C
	huffman.C)

if (LIBRA_HAVE_MPI)
   list(APPEND WAVELET_SOURCES
        wt_parallel.C	
        par_ezw_encoder.C)
endif()


add_library(wavelet ${WAVELET_SOURCES})
if (LIBRA_HAVE_MPI)
  target_link_libraries(wavelet ${MPI_LIBRARIES})
endif()
install(TARGETS wavelet DESTINATION lib)

include_directories(
  ${Boost_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}
  ${MPI_INCLUDE_PATH})


set(WAVELET_HEADERS 
	ac_obitstream.h
	ac_ibitstream.h
	buffered_obitstream.h
	buffered_ibitstream.h
	byte_budget_exception.h
	cdf97.h
	ezw.h
	ezw_encoder.h
	ezw_decoder.h
	filter_bank.h
	ibitstream.h
	io_utils.h
	matrix_utils.h
	obitstream.h
	stl_utils.h
	timing.h
	Timer.h
	vector_ibitstream.h
	vector_obitstream.h
	wavelet.h
	wt_1d.h
	wt_2d.h
	wt_direct.h
	wt_1d_lift.h
	wt_1d_direct.h
	wt_lift.h)

install(FILES ${WAVELET_HEADERS} DESTINATION include)
