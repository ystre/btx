cmake_minimum_required(VERSION 3.16)

project(btx
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---- libbtx ----
add_library(libbtx src/btx.c)
set_target_properties(libbtx PROPERTIES OUTPUT_NAME btx)
target_include_directories(libbtx PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ---- btx (CLI executable) ----
add_executable(btx src/main.c)
target_link_libraries(btx PRIVATE libbtx)
target_compile_definitions(btx PRIVATE BTX_VERSION="${PROJECT_VERSION}")

# ---- tests ----
enable_testing()
add_executable(test-btx tests/btx.test.c)
target_link_libraries(test-btx PRIVATE libbtx)
add_test(NAME test-btx COMMAND test-btx)

# ---- install ----
install(TARGETS libbtx btx
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)
