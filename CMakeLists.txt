cmake_minimum_required(VERSION 3.24)

project(btx
    VERSION 0.2.0
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/dependencies.cmake)

enable_testing()

add_subdirectory(btx)
add_subdirectory(btx-cli)

add_subdirectory(tests)
