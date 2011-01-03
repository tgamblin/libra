set(EFFORT_HEADERS
  effort_api.h
	effort_data.h
	effort_key.h
	effort_module.h
	effort_params.h
	effort_record.h
	effort_signature.h
	effort_dataset.h
	Metric.h)

set(EFFORT_NOINST_HEADERS 
	parallel_compressor.h
	parallel_decompressor.h
	env_config.h
	synchronize_keys.h
	FrameDB.h
	sampler.h
	ltqnorm.h
	s3d_topology.h)

install(FILES ${EFFORT_HEADERS} DESTINATION include)

set(EFFORT_SOURCES 
	effort_key.C
	effort_record.C
	effort_signature.C
	effort_data.C
	effort_params.C
	Metric.C
	FrameDB.C
	effort_dataset.C
	s3d_topology.C)

if (LIBRA_HAVE_MPI)
  list(APPEND EFFORT_SOURCES
    parallel_compressor.C
	  parallel_decompressor.C
	  synchronize_keys.C)
	
  if (HAVE_SPRNG)
    list(APPEND EFFORT_SOURCES
      sampler.C 
	    ltqnorm.C)
    
    set(SAMPLE_PROGS  
      sample-test 
	    approx-timer) 
  endif()
endif()

add_library(effort ${EFFORT_SOURCES})
target_link_libraries(effort wavelet callpath)
install(TARGETS effort DESTINATION lib)


#
# Effort runtime library and wrapper libraries
# TODO: make an include file or something for using wrap.py.
#
if (LIBRA_BUILD_EFFORT_RUNTIME)
  add_library(effort-runtime          effort_module.C env_config.C)
  target_link_libraries(effort-runtime effort)

  # add a custom build rule for generating wrapper files
  function(add_wrapper_file w_file)
    # autogenerate tools that come from .w files.
    string(REGEX REPLACE ".w$" ".C" output_src ${w_file})
    if (output_src)
      add_custom_command(
        OUTPUT ${output_src}
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/wrap.py ${w_file} -o ${output_src})
    endif()
  endfunction()
  add_wrapper_file(base_wrapper.w)
  add_wrapper_file(comm_wrapper.w)
  add_wrapper_file(effort_wrapper.w)


  # Add a PMPI tool library that just uses the effort runtime.
  # Also adds a PnMPI version of the tool
  function(add_effort_tool name src_name)
    add_library(${name} ${src_name} base_wrapper.C)
    target_link_libraries(${name} effort-runtime)
    
    add_library(${name}-mod MODULE ${src_name} base_wrapper.C)
    target_link_libraries(${name} effort-runtime)
  endfunction()

  add_effort_tool(manual-effort "")
  add_effort_tool(pmpi-effort   effort_wrapper.C)
  add_effort_tool(comm-effort   comm_wrapper.C)

  add_library(pcontrol-counter  pcontrol_counter.C)
  add_library(timing            timing_module.C)
endif()


# Simple function to add an executable and link it to effort library.
function(add_effort_program prog_name src_name)
  add_executable(${prog_name} ${src_name})
  target_link_libraries(${prog_name} effort ${MPI_LIBRARIES})
endfunction()

add_effort_program(nrmse nrmse.C)
add_effort_program(ef    ef.C)

if (LIBRA_HAVE_MPI)
  add_effort_program(tuner                  tuner.C)
  add_effort_program(bin-test               bin_test.C)
  add_effort_program(dataset-test           dataset_test.C)
	add_effort_program(effort-signature-test  effort_signature_test.C)
  add_effort_program(s3d-topo-test          s3d_topo_test.C)
 	add_effort_program(parse-callpath-test    parse_callpath_test.C)


  if (HAVE_MUSTER)
    function(add_muster_program prog_name src_name)
      add_effort_program(${prog_name} ${src_name})
      target_link_libraries(${prog_name} ${MUSTER_LIBRARIES})
    endfunction()
    
    add_muster_program(approx-timer               approx_timer.C)
    add_muster_program(sample-test                sample_test.C)
	  add_muster_program(signature-cluster-test     signature_cluster_test.C)
	  add_muster_program(par-signature-cluster-test signature_cluster_test.C)
  endif()
endif()

# Scripts to install
install(PROGRAMS libra-build-viewer-data libra-reconstruct-exact DESTINATION bin)

##
## This stuff is for generating PMPI wrapper libraries.
##
#set(CLEANFILES effort_wrapper.C comm_wrapper.C base_wrapper.C)
#set(EXTRA_DIST effort_wrapper.w comm_wrapper.w base_wrapper.w)
#
#$(srcdir)/base_wrapper.C: base_wrapper.w
#	$(PYTHON) $(srcdir)/wrap.py $< -o $@
#
#$(srcdir)/effort_wrapper.C: effort_wrapper.w
#	$(PYTHON) $(srcdir)/wrap.py $< -o $@
#
#$(srcdir)/comm_wrapper.C: comm_wrapper.w
#	$(PYTHON) $(srcdir)/wrap.py $< -o $@
#
#$(srcdir)/waitall_wrapper.C: waitall_wrapper.w
#	$(PYTHON) $(srcdir)/wrap.py $< -o $@

include_directories(
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/libwavelet
	${PROJECT_SOURCE_DIR}/callpath
  ${Boost_INCLUDE_DIR}
	${MPI_CXXFLAGS})
