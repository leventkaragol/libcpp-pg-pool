cmake_minimum_required(VERSION 3.14)

project(libcpp-pg-pool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(libcpp-pg-pool INTERFACE)

target_include_directories(libcpp-pg-pool INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(examples)
add_subdirectory(test)
