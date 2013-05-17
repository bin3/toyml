cmake_minimum_required(VERSION 2.8)

project(bindy CXX)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


set(root_dir ${PROJECT_SOURCE_DIR})
set(src_dir ${root_dir}/src)
set(build_dir ${root_dir}/build)
set(bin_dir ${root_dir}/bin)
set(lib_dir ${root_dir}/lib)
set(third_dir ${root_dir}/../third_party)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
  #set(CMAKE_BUILD_TYPE "Release")
endif()

set(CXX_FLAGS
  -g
  # -DVALGRIND
  -Wall
  -Wextra
  # -m32
  -Werror
  #-Wconversion
  -Wno-unused-parameter
  -Wold-style-cast
  -Woverloaded-virtual
  -Wpointer-arith
  -Wshadow
  -Wwrite-strings
  #-march=native
  # -MMD
  -std=c++0x
  -fopenmp
)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")

set(EXECUTABLE_OUTPUT_PATH ${bin_dir})
set(LIBRARY_OUTPUT_PATH ${lib_dir})

include_directories(
  ${src_dir}
  #${third_dir}/include
  ${Boost_INCLUDE_DIRS}
)
link_directories(
  ${lib_dir}
  #${third_dir}/lib
)

# lib
#set(Boost_DEBUG 1)
set(BOOST_ROOT ${third_dir})
set(Boost_USE_STATIC_LIBS on)
#set(Boost_USE_MULTITHREADED on)
#set(Boost_USE_STATIC_RUNTIME on)
find_package(Boost REQUIRED)
find_package(Boost 1.45.0 COMPONENTS thread system date_time)
message(STATUS "Boost_LIBRARIES=${Boost_LIBRARIES}")

set(toyml_libs toyml_tm)
set(basic_libs ${toyml_libs} ${Boost_LIBRARIES} glog gflags)
message(STATUS "basic_libs=${basic_libs}")
include(common)

add_subdirectory(src)
