cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -march=native)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  set(CMAKE_CXX_LINKER_LAUNCHER   "${CCACHE_PROGRAM}")
endif()

set(MAIN_CPP "${CMAKE_CURRENT_LIST_DIR}/src/main.cpp")

project(compression-benchmark)
file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
list(REMOVE_ITEM CPP_SOURCES "${MAIN_CPP}")
file(GLOB_RECURSE CPP_HEADERS "${CMAKE_CURRENT_LIST_DIR}/include/*.hpp")

add_library(compression-benchmark-library SHARED ${CPP_SOURCES} ${CPP_HEADERS})
target_include_directories(compression-benchmark-library PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
set_property(TARGET compression-benchmark-library PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET compression-benchmark-library PROPERTY OUTPUT_NAME "compression-benchmark")

add_executable(compression-benchmark-app "${MAIN_CPP}")
add_dependencies(compression-benchmark-app compression-benchmark-library)
target_link_libraries(compression-benchmark-app PRIVATE compression-benchmark-library)
set_property(TARGET compression-benchmark-app PROPERTY OUTPUT_NAME "compression-benchmark")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(compression-benchmark-library PUBLIC -fsanitize=address)
  target_link_options(compression-benchmark-library PUBLIC -fsanitize=address)
endif()

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

find_package (Eigen3 REQUIRED NO_MODULE)
target_link_libraries(compression-benchmark-library PRIVATE Eigen3::Eigen)

find_package (Snappy 1.1 REQUIRED NO_MODULE)
target_link_libraries(compression-benchmark-library PRIVATE Snappy::snappy)

include(ExternalProject)
ExternalProject_Add(libbsc
  GIT_REPOSITORY    https://github.com/IlyaGrebnov/libbsc.git
  GIT_TAG           baf06ad80d507d7edbe028c205a4b6883e0d981d
  CONFIGURE_COMMAND ""
  PATCH_COMMAND     bash -c "sed -i '/^CFLAGS += -fopenmp -DLIBBSC_OPENMP_SUPPORT$/ s/.*/# &\\nCFLAGS += -fPIC/' makefile"
  BUILD_IN_SOURCE   1
  INSTALL_COMMAND   ""
)
ExternalProject_Get_property(libbsc SOURCE_DIR)
target_include_directories(compression-benchmark-library PRIVATE ${SOURCE_DIR}/libbsc)
target_link_libraries(compression-benchmark-library PRIVATE ${SOURCE_DIR}/libbsc.a)
add_dependencies(compression-benchmark-library libbsc)

ExternalProject_Add(libsz3
  GIT_REPOSITORY    https://github.com/szcompressor/SZ3.git
  GIT_TAG           47593621ff40350f83d8791fd2ea2a1966782e6a
  BUILD_COMMAND     cmake --build . --target zstd
  INSTALL_COMMAND   ""
  CMAKE_ARGS        -DCMAKE_BUILD_TYPE=Release -DSZ3_USE_BUNDLED_ZSTD=ON
)
ExternalProject_Get_property(libsz3 SOURCE_DIR)
target_include_directories(compression-benchmark-library PRIVATE ${SOURCE_DIR}/include)
target_include_directories(compression-benchmark-library PRIVATE ${SOURCE_DIR}/tools/zstd/)
ExternalProject_Get_property(libsz3 BINARY_DIR)
target_include_directories(compression-benchmark-library PRIVATE ${BINARY_DIR}/include)
target_link_libraries(compression-benchmark-library PRIVATE ${BINARY_DIR}/tools/zstd/libzstd.so)
add_dependencies(compression-benchmark-library libsz3)

ExternalProject_Add(liblz4
  GIT_REPOSITORY    https://github.com/lz4/lz4.git
  GIT_TAG           5ff839680134437dbf4678f3d0c7b371d84f4964
  CONFIGURE_COMMAND ""
  BUILD_IN_SOURCE   1
  INSTALL_COMMAND   ""
)
ExternalProject_Get_property(liblz4 SOURCE_DIR)
target_include_directories(compression-benchmark-library PRIVATE ${SOURCE_DIR}/lib)
target_link_libraries(compression-benchmark-library PRIVATE ${SOURCE_DIR}/lib/liblz4.so)
add_dependencies(compression-benchmark-library liblz4)

ExternalProject_Add(machete
  GIT_REPOSITORY    https://github.com/mrmbernardi/Machete.git # https://github.com/Gyhanis/Machete.git
  GIT_TAG           556043d920bfe9ffe0b78bdfe5641cbfbf21d158
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     CFLAG=-fpic make # MODE=DEBUG
  BUILD_IN_SOURCE   1
  INSTALL_COMMAND   ""
)
ExternalProject_Get_property(machete SOURCE_DIR)
target_include_directories(compression-benchmark-library PRIVATE ${SOURCE_DIR}/machete)
target_link_libraries(compression-benchmark-library PRIVATE ${SOURCE_DIR}/lib/libmach.a)
add_dependencies(compression-benchmark-library machete)

ExternalProject_Add(tabulate
  GIT_REPOSITORY    https://github.com/p-ranav/tabulate.git
  GIT_TAG           b35db4cce50a4b296290b0ae827305cdeb23751e
  INSTALL_COMMAND   ""
)
ExternalProject_Get_property(tabulate SOURCE_DIR)
target_include_directories(compression-benchmark-library PRIVATE ${SOURCE_DIR}/include)
target_include_directories(compression-benchmark-app PRIVATE ${SOURCE_DIR}/include)
add_dependencies(compression-benchmark-library tabulate)
add_dependencies(compression-benchmark-app tabulate)

ExternalProject_Add(pcodec
  GIT_REPOSITORY    https://github.com/mwlon/pcodec.git
  GIT_TAG           3450aa6d825c142af5150a6ee016605a2de9a125
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     cargo build --release
  BUILD_IN_SOURCE   1
  INSTALL_COMMAND   ""
)
ExternalProject_Get_property(pcodec SOURCE_DIR)
target_include_directories(compression-benchmark-library PRIVATE ${SOURCE_DIR}/pco_c/include)
target_link_libraries(compression-benchmark-library PRIVATE ${SOURCE_DIR}/target/release/libcpcodec.so)
add_dependencies(compression-benchmark-library pcodec)