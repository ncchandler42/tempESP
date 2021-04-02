cmake_minimum_required(VERSION 3.13)

project(tempesp_srv)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -O3 -g)

find_package(rtlsdr REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(include)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(tempesp_train 
	src/tempesp_train.cpp
)

target_link_libraries(tempesp_train ${OpenCV_LIBS})
target_link_libraries(tempesp_train rtlsdr)
