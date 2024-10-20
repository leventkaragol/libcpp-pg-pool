cmake_minimum_required(VERSION 3.14)

project(examples)

find_package(libpqxx CONFIG REQUIRED)

add_executable(examples main.cpp)

target_link_libraries(examples PRIVATE libcpp-pg-pool libpqxx::pqxx)
