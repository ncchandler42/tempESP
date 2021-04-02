cmake_minimum_required(VERSION 3.5)

project(tempesp_cli)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(include)
add_compile_options(-Wall -O3 -g)

add_executable(tempesp_cli src/tempesp_cli.cpp)
target_link_libraries( tempesp_cli ${OpenCV_LIBS} )
