cmake_minimum_required (VERSION 2.8)
project (SHARPSAT) 

option(STATIC "STATIC" ON)
set(CMAKE_VERBOSE_MAKEFILE on)

if(STATIC)
set(CMAKE_EXE_LINKER_FLAGS -static)
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
set(CMAKE_SHARED_LIBRARY_C_FLAGS)
set(CMAKE_SHARED_LIBRARY_CXX_FLAGS)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
set(BUILD_SHARED_LIBS OFF)
set(Boost_USE_STATIC_LIBS ON)
endif(STATIC)

set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -Wall ")

SET(CMAKE_C_FLAGS "-std=c99 -Wall -msse4.1 -mpclmul")

find_library(GMP_LIB gmp)
find_library(GMPXX_LIB gmpxx)
find_library(MPFR_LIB mpfr)
find_package(Boost REQUIRED process)

add_definitions(-DMPFR_USE_NO_MACRO)
add_subdirectory(flowcutter)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/flowcutter)

SET(SOURCES 
	src/preprocessor/graph.cpp 
	src/preprocessor/subsumer.cpp
	src/preprocessor/treewidth.cpp
	src/preprocessor/twpp.cpp
	src/clhash/clhash.c
)

add_executable (sharpSAT src/main.cpp ${SOURCES})
if(STATIC)
SET_TARGET_PROPERTIES (sharpSAT PROPERTIES LINK_SEARCH_START_STATIC 1)
endif(STATIC)
target_link_libraries (sharpSAT ${MPFR_LIB} ${GMPXX_LIB} ${GMP_LIB} ${Boost_LIBRARIES})

add_executable (flow_cutter_pace17 flowcutter/pace.cpp)
if(STATIC)
SET_TARGET_PROPERTIES (flow_cutter_pace17 PROPERTIES LINK_SEARCH_START_STATIC 1)
endif(STATIC)
target_link_libraries (flow_cutter_pace17 libflowcutter)
