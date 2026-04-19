cmake_minimum_required(VERSION 3.10)
project(RpiVisionApp)

set(CMAKE_CXX_STANDARD 11)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(my_vision_app main.cpp)
target_link_libraries(my_vision_app Qt5::Widgets ${OpenCV_LIBS})