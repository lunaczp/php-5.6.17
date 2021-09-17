cmake_minimum_required(VERSION 3.6)
project(php_5_6_17)

set(CMAKE_C_STANDARD 99)

#add_executable(php_5_6_17 ${SOURCE_FILES})
add_custom_target(php_5_6_17 COMMAND make -C ${php_5_6_17_SOURCE_DIR})
