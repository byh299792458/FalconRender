cmake_minimum_required(VERSION 3.7)

project(FalconRendererExamples)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../FalconRender)

find_package(OpenMP)
if (OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()

add_custom_target(CopyDLL ALL)
add_custom_command(TARGET CopyDLL
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/assimp/bin/assimp-vc140-mt.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/assimp/bin/assimp-vc140-mt.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2_image/lib/x64/libjpeg-9.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2_image/lib/x64/libjpeg-9.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2_image/lib/x64/libpng16-16.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2_image/lib/x64/libpng16-16.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2_image/lib/x64/libtiff-5.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2_image/lib/x64/libtiff-5.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2_image/lib/x64/libwebp-7.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2_image/lib/x64/libwebp-7.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2/lib/x64/SDL2.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2/lib/x64/SDL2.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2_image/lib/x64/SDL2_image.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2_image/lib/x64/SDL2_image.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2_image/lib/x64/zlib1.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		${PROJECT_SOURCE_DIR}/../../depend/sdl2_image/lib/x64/zlib1.dll 
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release
)

add_executable(BoxTest box_test.cpp model.hpp timer.hpp mvp_matrices.hpp)
target_link_libraries(BoxTest FalconRenderer ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
	${ASSIMP_LIB})

add_executable(TriangleTest triangle_test.cpp model.hpp timer.hpp mvp_matrices.hpp)
target_link_libraries(TriangleTest FalconRenderer ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
	${ASSIMP_LIB})

