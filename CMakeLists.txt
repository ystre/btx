cmake_minimum_required(VERSION 3.16)

project(btx
    VERSION 0.3.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(libbtx)
add_subdirectory(btx)
