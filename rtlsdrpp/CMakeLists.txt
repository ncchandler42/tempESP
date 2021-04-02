cmake_minimum_required(VERSION 3.10)

project(sdr-test)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
add_compile_options(-Wall -O3 -g)

find_package(rtlsdr)

add_executable(test src/test.cpp)

target_link_libraries(test PUBLIC rtlsdr)
