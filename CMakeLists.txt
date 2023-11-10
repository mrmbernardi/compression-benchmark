cmake_minimum_required(VERSION 3.22)

include(ExternalProject)

project(compression-benchmark)
add_executable(compression-benchmark src/compression-benchmark.cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message( FATAL_ERROR "To support libbsc, Clang is required. Set -DCMAKE_CXX_COMPILER=clang++" )
endif()

ExternalProject_Add(libbsc
  GIT_REPOSITORY    git@github.com:IlyaGrebnov/libbsc.git
  GIT_TAG           v3.3.2
  CONFIGURE_COMMAND ""
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND ""
)
ExternalProject_Get_property(libbsc SOURCE_DIR)
include_directories(${SOURCE_DIR}/libbsc)
target_link_libraries(compression-benchmark PRIVATE ${SOURCE_DIR}/libbsc.a)
find_package(OpenMP REQUIRED)
target_link_libraries(compression-benchmark PUBLIC OpenMP::OpenMP_CXX)
add_dependencies(compression-benchmark libbsc)