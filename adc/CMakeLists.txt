cmake_minimum_required(VERSION 2.6)
project(bat_recorder)

#Version number:
set (bat_recorder_VERSION_MAJOR 0)
set (bat_recorder_VERSION_MINOR 1)
#Configure a header file to pass CMake stuff to the source code:
configure_file (
	${PROJECT_SOURCE_DIR}/src/bat.h.in
	${PROJECT_BINARY_DIR}/bat.h
	)
#------------------------------------------------
#Add all executables:
#------------------------------------------------
#General files:
set (GENERAL	
	src/control.cpp
	#src/recorder.h
	src/recorder.hpp
	src/c_buffer.hpp
	src/utility.h
)

#------------------------------------------------
#Set subdirectories:
#------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable(snapshot ${GENERAL})
#------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(snapshot -lcomedi -lm)
#------------------------------------------------
#Set compiler flags
#------------------------------------------------

set(CMAKE_CXX_FLAGS "-std=c++0x -O3 -pthread -Wall")

