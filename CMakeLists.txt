cmake_minimum_required (VERSION 3.18)
project (fpc)

add_executable (
  fpc-test
  test.c
)

target_include_directories (
  fpc-test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)