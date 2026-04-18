add_executable(btx main.c)
target_link_libraries(btx PRIVATE libbtx)
target_compile_definitions(btx PRIVATE BTX_VERSION="${PROJECT_VERSION}")

install(TARGETS btx RUNTIME DESTINATION bin)
