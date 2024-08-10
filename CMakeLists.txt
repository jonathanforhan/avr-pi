cmake_minimum_required(VERSION 3.20)
project(avr-pi VERSION 0.0.0 LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(AVR_PI_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -flto")
set(AVR_PI_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/avr.c")
set(AVR_PI_PUB_INC "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(AVR_PI_PRIV_INC "${CMAKE_CURRENT_SOURCE_DIR}/src")

# avr-pi lib
add_library(avr-pi-lib STATIC "${AVR_PI_SRCS}")
target_include_directories(avr-pi-lib PUBLIC "${AVR_PI_PUB_INC}" PRIVATE "${AVR_PI_PRIV_INC}")
set_target_properties(avr-pi-lib PROPERTIES COMPILE_FLAGS "${AVR_PI_FLAGS}")

# avr-pi cli
add_executable(avr-pi "${CMAKE_CURRENT_SOURCE_DIR}/src/pi.c")
if(pigpio)
    target_link_libraries(avr-pi PRIVATE avr-pi-lib pigpio)
else()
    target_link_libraries(avr-pi PRIVATE avr-pi-lib)
    message(WARNING "pigpio not found! this should only be the case for testing")
endif()

# avr-pi tests
enable_testing()
add_subdirectory("test")
