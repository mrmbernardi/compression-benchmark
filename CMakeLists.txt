cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -march=native)

project(compression-benchmark)
file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
file(GLOB_RECURSE CPP_HEADERS "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp")
add_executable(compression-benchmark ${CPP_SOURCES} ${CPP_HEADERS})

set_source_files_properties(
  ${CPP_SOURCES}
  PROPERTIES
  COMPILE_FLAGS -Wall -Wextra -Wpedantic
)
set_source_files_properties(
  src/method_sz3.cpp
  PROPERTIES
  COMPILE_FLAGS -Wno-all -Wno-extra -Wno-pedantic
)

include(ExternalProject)
ExternalProject_Add(libbsc
  GIT_REPOSITORY    https://github.com/IlyaGrebnov/libbsc.git
  GIT_TAG           baf06ad80d507d7edbe028c205a4b6883e0d981d
  CONFIGURE_COMMAND ""
  PATCH_COMMAND     bash -c "sed -i '/^CFLAGS += -fopenmp -DLIBBSC_OPENMP_SUPPORT$/ s/./# &/' makefile"
  BUILD_IN_SOURCE   1
  INSTALL_COMMAND   ""
)
ExternalProject_Get_property(libbsc SOURCE_DIR)
target_include_directories(compression-benchmark PRIVATE ${SOURCE_DIR}/libbsc)
target_link_libraries(compression-benchmark PRIVATE ${SOURCE_DIR}/libbsc.a)
add_dependencies(compression-benchmark libbsc)

ExternalProject_Add(libsz3
  GIT_REPOSITORY    https://github.com/szcompressor/SZ3.git
  GIT_TAG           162674f8743b8a6ff2042ef9236471a772bdbd3f
  INSTALL_COMMAND   ""
  CMAKE_ARGS        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSZ3_USE_BUNDLED_ZSTD=ON
)
ExternalProject_Get_property(libsz3 SOURCE_DIR)
target_include_directories(compression-benchmark PRIVATE ${SOURCE_DIR}/include)
target_include_directories(compression-benchmark PRIVATE ${SOURCE_DIR}/tools/zstd/)
ExternalProject_Get_property(libsz3 BINARY_DIR)
target_include_directories(compression-benchmark PRIVATE ${BINARY_DIR}/include)
target_link_libraries(compression-benchmark PRIVATE ${BINARY_DIR}/tools/zstd/libzstd.so)
add_dependencies(compression-benchmark libsz3)

ExternalProject_Add(tabulate
  GIT_REPOSITORY    https://github.com/p-ranav/tabulate.git
  GIT_TAG           b35db4cce50a4b296290b0ae827305cdeb23751e
  INSTALL_COMMAND   ""
)
ExternalProject_Get_property(tabulate SOURCE_DIR)
target_include_directories(compression-benchmark PRIVATE ${SOURCE_DIR}/include)
add_dependencies(compression-benchmark tabulate)