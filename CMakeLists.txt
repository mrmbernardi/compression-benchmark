cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(compression-benchmark)
file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
file(GLOB_RECURSE CPP_HEADERS "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp")
add_executable(compression-benchmark ${CPP_SOURCES} ${CPP_HEADERS})

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message( FATAL_ERROR "To support libbsc, Clang is required. Set -DCMAKE_CXX_COMPILER=clang++" )
endif()

set_source_files_properties(
  ${CPP_SOURCES}
  PROPERTIES
  COMPILE_FLAGS -Wall -Wextra -Wpedantic
)

include(ExternalProject)
ExternalProject_Add(libbsc
  GIT_REPOSITORY    https://github.com/IlyaGrebnov/libbsc.git
  GIT_TAG           baf06ad80d507d7edbe028c205a4b6883e0d981d
  CONFIGURE_COMMAND ""
  BUILD_IN_SOURCE   1
  INSTALL_COMMAND   ""
)
ExternalProject_Get_property(libbsc SOURCE_DIR)
target_include_directories(compression-benchmark PRIVATE ${SOURCE_DIR}/libbsc)
target_link_libraries(compression-benchmark PRIVATE ${SOURCE_DIR}/libbsc.a)
find_package(OpenMP REQUIRED)
target_link_libraries(compression-benchmark PUBLIC OpenMP::OpenMP_CXX)
add_dependencies(compression-benchmark libbsc)

ExternalProject_Add(libsz3
  GIT_REPOSITORY    https://github.com/szcompressor/SZ3.git
  GIT_TAG           162674f8743b8a6ff2042ef9236471a772bdbd3f
  INSTALL_COMMAND   ""
  CMAKE_ARGS        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSZ3_USE_BUNDLED_ZSTD=ON
)
ExternalProject_Get_property(libsz3 SOURCE_DIR)
target_include_directories(compression-benchmark PRIVATE ${SOURCE_DIR}/include)
ExternalProject_Get_property(libsz3 BINARY_DIR)
target_include_directories(compression-benchmark PRIVATE ${BINARY_DIR}/include)
target_link_libraries(compression-benchmark PRIVATE ${BINARY_DIR}/tools/zstd/libzstd.so)
add_dependencies(compression-benchmark libsz3)
