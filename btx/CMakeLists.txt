add_executable(btx main.cpp)
target_link_libraries(btx PRIVATE libbtx)
target_compile_definitions(btx PRIVATE BTX_VERSION="${PROJECT_VERSION}-${GIT_HASH}")

install(TARGETS btx RUNTIME DESTINATION bin)
