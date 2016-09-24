cmake_minimum_required(VERSION 3.0)

project(LocalTester C CXX)

include(ExternalProject)

find_package(Git REQUIRED)
find_package(OpenCV REQUIRED)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../cpp_client")

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../out-libs")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/")

add_executable(LocalTester main.cpp)

target_link_libraries(LocalTester ${OpenCV_LIBS} CppClient)