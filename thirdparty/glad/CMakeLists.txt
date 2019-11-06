cmake_minimum_required(VERSION 3.9)
project(glad)


set(HEADER_FILES
	include/glad/glad.h
	include/KHR/khrplatform.h
)

set(SOURCE_FILES
	src/glad.c
)

add_library(${PROJECT_NAME} STATIC
			${HEADER_FILES}
			${SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
						   ${CMAKE_CURRENT_LIST_DIR}/include
)
				
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /MP")
endif(MSVC)