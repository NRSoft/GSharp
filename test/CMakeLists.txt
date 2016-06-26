set (GSharp_TEST
  ${PROJECT_SOURCE_DIR}/src/gsharp_parser.cpp
  ${PROJECT_SOURCE_DIR}/src/gsharp_program.cpp
  ${CMAKE_CURRENT_LIST_DIR}/parse_expression_test.cpp
  ${CMAKE_CURRENT_LIST_DIR}/parse_o_code_test.cpp
  )

# googletest headers and libraries
include_directories ("${PROJECT_SOURCE_DIR}/external/gtest/include")
set (GTEST_LIB_DIR ${PROJECT_SOURCE_DIR}/external/gtest/lib)

# Requires TEST_BUILD for proper testing
add_definitions (-DTEST_BUILD)

# add test executable 
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
add_executable (gsharp_test ${GSharp_TEST})
target_link_libraries (gsharp_test ${GTEST_LIB_DIR}/libgtest.a)
target_link_libraries (gsharp_test ${GTEST_LIB_DIR}/libgtest_main.a)
target_link_libraries (gsharp_test pthread)
