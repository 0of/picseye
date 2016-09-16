cmake_minimum_required(VERSION 3.0)

project(PicsJni C CXX)
include(ExternalProject)

find_package(JNI REQUIRED)

include_directories(
  ${JAVA_INCLUDE_PATH}
  ${JAVA_INCLUDE_PATH2}
  target/include
  "${CMAKE_CURRENT_BINARY_DIR}/../cpp_client")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -O2 -std=c++14")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/target)

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../out-libs/")

add_library(picsjni SHARED ${CMAKE_SOURCE_DIR}/src/main/jni/PicsClient.cpp)

target_link_libraries(picsjni CppClient)