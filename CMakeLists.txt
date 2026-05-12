cmake_minimum_required(VERSION 3.24)

project(btx
    VERSION 0.5.0
    LANGUAGES C CXX
)

include(cmake/compiler-warnings.cmake)
include(cmake/compiler-settings.cmake)

execute_process(
    COMMAND git -C ${CMAKE_SOURCE_DIR} describe --always --dirty
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

enable_testing()

add_subdirectory(libbtx)
add_subdirectory(btx)
add_subdirectory(tests)
