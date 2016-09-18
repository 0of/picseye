cmake_minimum_required(VERSION 3.0)

project(DescImport C CXX)

include(ExternalProject)

find_package(Git REQUIRED)
find_package(OpenCV REQUIRED)

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

ExternalProject_Get_Property(leveldb_proj source_dir)
set(leveldb_src_dir ${source_dir})

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${leveldb_src_dir}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/bktree_leveldb/src")

link_directories("${leveldb_src_dir}/out-static")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/")

add_executable(DescImport main.cpp)

add_dependencies(DescImport leveldb_proj)

target_link_libraries(DescImport leveldb ${OpenCV_LIBS})