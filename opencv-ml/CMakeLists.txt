cmake_minimum_required( VERSION 2.8 )
project( opencv-ml )

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable( opencv-ml src/opencv-ml.cpp )
add_executable( points_classifier src/points_classifier.cpp )
target_link_libraries( opencv-ml ${OpenCV_LIBS} )
target_link_libraries( points_classifier ${OpenCV_LIBS} )
