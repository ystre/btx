add_library(libbtx btx.c)
set_target_properties(libbtx PROPERTIES OUTPUT_NAME btx)
target_include_directories(libbtx PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

code_analysis(libbtx PRIVATE)

add_executable(test-btx btx.test.c)
target_link_libraries(test-btx PRIVATE libbtx)
add_test(NAME test-btx COMMAND test-btx)

install(TARGETS libbtx ARCHIVE DESTINATION lib)
install(FILES btx.h DESTINATION include/libbtx)
