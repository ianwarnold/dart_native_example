cmake_minimum_required(VERSION 3.13)
project(dart_native_example)

set(CMAKE_CXX_STANDARD 11)

set(DART_INCLUDE_DIR "" CACHE PATH "Location of Dart SDK include (.h) files.")

add_library(sample_extension SHARED sample_extension.cpp sample_extension.h)

target_include_directories(sample_extension PUBLIC ${DART_INCLUDE_DIR})
target_compile_definitions(sample_extension PUBLIC -DDART_SHARED_LIB)