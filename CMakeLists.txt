cmake_minimum_required (VERSION 3.1.0 FATAL_ERROR)
project (GSharp VERSION 1.0)

## Set the build type
set (CMAKE_BUILD_TYPE Release)

## C++ compiler options
set (CMAKE_CXX_FLAGS "-Wall -std=c++11" )
set (CMAKE_CXX_FLAGS_DEBUG "-g -O0" )
set (CMAKE_CXX_FLAGS_RELEASE "-O3" )

# add the main library
include_directories ("${PROJECT_SOURCE_DIR}/include")
add_subdirectory (${PROJECT_SOURCE_DIR}/src)
set (EXTRA_LIBS ${EXTRA_LIBS} gsharp)

# add the executable 
add_executable (gs2g example.cpp)
target_link_libraries (gs2g  ${EXTRA_LIBS})

# add unit tests
add_subdirectory (${PROJECT_SOURCE_DIR}/external/gtest)
enable_testing ()
add_subdirectory (${PROJECT_SOURCE_DIR}/test)
add_test (NAME Test COMMAND gsharp_test)
