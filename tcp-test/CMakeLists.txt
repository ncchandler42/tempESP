cmake_minimum_required(VERSION 3.5)

project(tcp-test)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
add_compile_options(-Wall -O3 -g)

add_executable(cli src/cli.cpp)
add_executable(srv src/srv.cpp)
