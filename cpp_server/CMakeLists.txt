cmake_minimum_required(VERSION 3.0)

project(CppServer C CXX)

include(ExternalProject)

find_package(Git REQUIRED)

find_path ( ZMQ_INCLUDE_DIR zmq.h )
find_library ( ZMQ_LIBRARY NAMES zmq )

ExternalProject_Add(
    leveldb_proj
    PREFIX ${CMAKE_BINARY_DIR}/deps
    GIT_REPOSITORY https://github.com/google/leveldb.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)

ExternalProject_Add(
    cppzmq_proj
    PREFIX ${CMAKE_BINARY_DIR}/deps
    GIT_REPOSITORY https://github.com/zeromq/cppzmq.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    LOG_DOWNLOAD ON
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_COMMAND ""
)

ExternalProject_Get_Property(cppzmq_proj source_dir)
set(cppzmq_src_dir ${source_dir})

ExternalProject_Get_Property(leveldb_proj source_dir)
set(leveldb_src_dir ${source_dir})

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${cppzmq_src_dir}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../third_party/bktree_leveldb/src")
include_directories("${leveldb_src_dir}/include")
include_directories("${ZMQ_INCLUDE_DIR}")

link_directories("${leveldb_src_dir}/out-static")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_BUILD_TYPE Release)

add_executable(CppServer main.cpp) 

add_dependencies(CppServer leveldb_proj cppzmq_proj)

target_link_libraries(CppServer leveldb ${ZMQ_LIBRARY})