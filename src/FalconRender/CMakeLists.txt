cmake_minimum_required(VERSION 3.7)

project(FalconRenderer)

aux_source_directory(${CMAKE_SOURCE_DIR}/src/FalconRender SOURCE_FILES)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/src/FalconRender/*.hpp)

find_package(OpenMP)
if (OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)
	add_definitions("-Wall")
endif ()

add_library(FalconRenderer ${SOURCE_FILES} ${HEADER_FILES})