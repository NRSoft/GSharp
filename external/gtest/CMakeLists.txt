include_directories ("${CMAKE_CURRENT_LIST_DIR}")
include_directories ("${CMAKE_CURRENT_LIST_DIR}/include")

# build gtest library
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/lib)
add_library (gtest STATIC ${CMAKE_CURRENT_LIST_DIR}/src/gtest-all.cc)
add_library (gtest_main STATIC ${CMAKE_CURRENT_LIST_DIR}/src/gtest_main.cc)