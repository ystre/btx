set(TEST_SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

function(add_btx_script_test name script)
    add_test(NAME ${name}
        COMMAND bash "${TEST_SCRIPTS_DIR}/${script}/driver.sh")
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "BUILD_DIR=${CMAKE_BINARY_DIR}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endfunction()

add_btx_script_test(encode   encode)
add_btx_script_test(decode   decode)
add_btx_script_test(validate validate)
add_btx_script_test(roundtrip roundtrip)
add_btx_script_test(errors   errors)
