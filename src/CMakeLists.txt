set (GSharp_SOURCE
  ${CMAKE_CURRENT_LIST_DIR}/gsharp_program.cpp
  ${CMAKE_CURRENT_LIST_DIR}/gsharp_parser.cpp
  ${CMAKE_CURRENT_LIST_DIR}/gsharp.cpp
  )

# build the library
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
add_library (gsharp STATIC ${GSharp_SOURCE})