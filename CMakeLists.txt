cmake_minimum_required(VERSION 3.9)
project(beats)

set(GLM_TEST_ENABLE OFF)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/glfw-3.3/)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/glad/)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/glm/)

set(THIRDPARTY_HEADERS
	thirdparty/FFTW/fftw3.h
)

set(VERTEX_SHADERS
	shaders/bar.vs
)

set(FRAGMENT_SHADERS
	shaders/bar.fs
)

set(HEADER_FILES
	include/KeyPressWatcher.h
	include/Shader.h
	include/SpectrogramBar.h
	include/Utilities.h
)

set(SOURCE_FILES
	src/KeyPressWatcher.cpp
	src/main.cpp
	src/Shader.cpp
	src/SpectrogramBar.cpp
	src/Utilities.cpp
)

add_executable(${PROJECT_NAME}
			   ${THIRDPARTY_HEADERS}
			   ${HEADER_FILES}
			   ${SOURCE_FILES}
			   ${VERTEX_SHADERS}
			   ${FRAGMENT_SHADERS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
						   ${CMAKE_CURRENT_LIST_DIR}/include
						   $ENV{FMOD_DIR}/core/inc/
						   $ENV{FMOD_DIR}/studio/inc/
						   ${CMAKE_CURRENT_LIST_DIR}/thirdparty/FFTW/
						   ${CMAKE_CURRENT_LIST_DIR}/thirdparty/glm/glm/
						   ${CMAKE_CURRENT_LIST_DIR}/thirdparty/glfw-3.3/include/
						   ${CMAKE_CURRENT_LIST_DIR}/thirdparty/glad/include/
)

target_link_libraries(${PROJECT_NAME} 
					  $ENV{FMOD_DIR}/core/lib/x64/fmod_vc.lib
					  $ENV{FMOD_DIR}/studio/lib/x64/fmodstudio_vc.lib
					  ${CMAKE_CURRENT_LIST_DIR}/thirdparty/FFTW/libfftw3-3.lib
					  ${CMAKE_CURRENT_LIST_DIR}/out/thirdparty/glm/glm/$<CONFIG>/glm_static.lib
					  ${CMAKE_CURRENT_LIST_DIR}/out/thirdparty/glfw-3.3/src/$<CONFIG>/glfw3.lib
					  ${CMAKE_CURRENT_LIST_DIR}/out/thirdparty/glad/$<CONFIG>/glad.lib
)

add_dependencies(${PROJECT_NAME} glfw)

source_group("thirdparty" FILES ${THIRDPARTY_HEADERS})
source_group("shaders" FILES ${VERTEX_SHADERS} ${FRAGMENT_SHADERS})
				
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

# Parallel compilation and C++17
if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /MP")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4554")
endif(MSVC)


if(WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{FMOD_DIR}/core/lib/x64/fmod.dll" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{FMOD_DIR}/studio/lib/x64/fmodstudio.dll" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/thirdparty/FFTW/libfftw3-3.dll" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/"
		VERBATIM
	)
endif(WIN32)


# Clang format setup
# http://mariobadr.com/using-clang-format-to-enforce-style.html
find_program(CLANG_FORMAT_EXE NAMES "clang-format")
if(NOT CLANG_FORMAT_EXE)
	message(STATUS "clang-format not found.")
else()
	message(STATUS "clang-format found: ${CLANG_FORMAT_EXE}")
	set(DO_CLANG_FORMAT "${CLANG_FORMAT_EXE}" "-i -style=file")
endif()

function(prepend var prefix)
	set(listVar "")
	
	foreach(f ${ARGN})
	  list(APPEND listVar "${prefix}/${f}")
	endforeach()
	
	set(${var} "${listVar}" PARENT_SCOPE)
endfunction()

if(CLANG_FORMAT_EXE)
	prepend(FILES_TO_FORMAT ${CMAKE_CURRENT_SOURCE_DIR} ${HEADER_FILES} ${SOURCE_FILES})
	
	add_custom_target(
		CLANG_FORMAT
		COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${FILES_TO_FORMAT}
	)
endif()