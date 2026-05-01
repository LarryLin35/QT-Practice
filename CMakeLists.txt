cmake_minimum_required(VERSION 3.10)
project(RpiVisionApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(my_vision_app
    main.cpp
    camera/cameraworker.cpp
    camera/cameraworker.h
    processor/circleprocessor.cpp
    processor/circleprocessor.h
    storage/framequeue.cpp
    storage/framequeue.h
    ui/mainwindow.cpp
    ui/mainwindow.h
)

target_include_directories(my_vision_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(my_vision_app Qt5::Core Qt5::Widgets ${OpenCV_LIBS})
