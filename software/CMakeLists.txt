#set min version
cmake_minimum_required(VERSION 3.13)

# Pull in SDK(must be before project)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
#set project name and C/C++ standard version
project(westinghouse12k C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#Initialize the SDK
pico_sdk_init()

#build files
add_executable(westinghouse12k Program.cpp )

#pull in common dependencies
target_link_libraries(westinghouse12k pico_stdlib)

# enable uart output, disable usb output
pico_enable_stdio_usb(westinghouse12k 1)
pico_enable_stdio_uart(westinghouse12k 1)

# create map / bin / hex file etc.
pico_add_extra_outputs(westinghouse12k)

