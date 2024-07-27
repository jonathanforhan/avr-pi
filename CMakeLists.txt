cmake_minimum_required(VERSION 3.20)
project(avr-pi VERSION 0.0.0 LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(AVR_PI_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=gnu99")
set(AVR_PI_INCS "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}src")
set(AVR_PI_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/avr.c")

# avr-pi lib
add_library(avr-pi-lib STATIC "${AVR_PI_SRCS}")
target_include_directories(avr-pi-lib PUBLIC "${AVR_PI_INCS}")
set_target_properties(avr-pi-lib PROPERTIES COMPILE_FLAGS "${AVR_PI_FLAGS}")

# avr-pi cli
add_executable(avr-pi "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
target_link_libraries(avr-pi PRIVATE avr-pi-lib)

# avr-pi test-lib
add_library(avr-pi-test-lib STATIC "${AVR_PI_SRCS}")
target_include_directories(avr-pi-test-lib PUBLIC "${AVR_PI_INCS}")
set_target_properties(avr-pi-test-lib PROPERTIES COMPILE_FLAGS "${AVR_PI_FLAGS}")
target_compile_definitions(avr-pi-test-lib PUBLIC "AVR_TEST")

# avr-pi tests
enable_testing()
add_subdirectory("test")
