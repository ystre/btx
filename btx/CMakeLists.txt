add_executable(btx main.cpp)
target_link_libraries(btx PRIVATE libbtx Boost::program_options)
target_compile_definitions(btx PRIVATE BTX_VERSION="${PROJECT_VERSION}-${GIT_HASH}")

install(TARGETS btx RUNTIME DESTINATION bin)
