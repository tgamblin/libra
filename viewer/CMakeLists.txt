set(VTK_WRAP_PYTHON_FIND_LIBS TRUE)
include("${VTK_DIR}/UseVTK.cmake")
include("${VTK_CMAKE_DIR}/vtkWrapPython.cmake")

# Avoid annoying warnings about VTK headers.  I know they're antequated, but I can't fix VTK.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")

set(VTKEFFORT_SOURCES 
  EffortData.C
  summary.C
  wavelet_ssim.C)
set(VTKEFFORT_WRAPPED_SOURCES
  vtkEffortData.C 
  vtkEffortSource.C 
  vtkZValueScalars.C)
vtk_wrap_python3(vtkeffort VTKEFFORT_WRAPPED_SOURCES ${VTKEFFORT_WRAPPED_SOURCES})
add_library(vtkeffort MODULE ${VTKEFFORT_SOURCES} ${VTKEFFORT_WRAPPED_SOURCES})
target_link_libraries(vtkeffort effort wavelet vtkRendering vtkCommonPythonD ${VTK_PYTHON_LIBRARIES})

include_directories(
  ${VTK_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/viewer
	${PROJECT_SOURCE_DIR}/libwavelet
	${PROJECT_SOURCE_DIR}/callpath
	${PROJECT_SOURCE_DIR}/effort)
