cmake_minimum_required (VERSION 3.18)
project (fpc)

include_directories (
  ${CMAKE_CURRENT_SOURCE_DIR}
)

option (FPC_BUILD_TEST "Whether to build tests." ON)

if (FPC_BUILD_TEST)
  add_subdirectory (test)
endif ()