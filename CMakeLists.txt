cmake_minimum_required(VERSION 3.7)

project("FalconRender")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_CXX_STANDARD 17)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

find_package(EIGEN3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/depend/sdl2/include)
include_directories(${PROJECT_SOURCE_DIR}/depend/sdl2_image/include)
include_directories(${PROJECT_SOURCE_DIR}/depend/assimp/include)
set(SDL2_LIBRARIES 
	${PROJECT_SOURCE_DIR}/depend/sdl2/lib/x64/SDL2.lib
	${PROJECT_SOURCE_DIR}/depend/sdl2/lib/x64/SDL2main.lib
	${PROJECT_SOURCE_DIR}/depend/sdl2/lib/x64/SDL2test.lib)
set(SDL2_IMAGE_LIBRARIES ${PROJECT_SOURCE_DIR}/depend/sdl2_image/lib/x64/SDL2_image.lib)
set(ASSIMP_LIB ${PROJECT_SOURCE_DIR}/depend/assimp/lib/assimp-vc140-mt.lib)

if (EXISTS "${CMAKE_BINARY_DIR}/CMakeTools/CMakeToolsHelpers.cmake")
	include(${CMAKE_BINARY_DIR}/CMakeTools/CMakeToolsHelpers.cmake)
endif ()

#find_package(Doxygen)
#if (DOXYGEN_FOUND)
#	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
#	add_custom_target(doc
#		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
#		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#		COMMENT "Generating API documentation with Doxygen" VERBATIM
#	)
#endif ()

add_subdirectory(src)
