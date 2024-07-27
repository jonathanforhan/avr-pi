set(TEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../src/test.c")

add_executable(avr-pi-test "${TEST_SRC}")
target_link_libraries(avr-pi-test PRIVATE avr-pi-test-lib)
add_test(NAME avr-pi-test COMMAND avr-pi-test)

add_custom_command(
    TARGET avr-pi-test
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_CTEST_COMMAND} -R "^avr-pi-test$" --output-on-failure
)
