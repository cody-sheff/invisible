cmake_minimum_required(VERSION 3.1...3.28)

include_directories(include)

set(CMAKE_BUILD_TYPE Release)
add_executable(takehome src/frame_processing_system/main.cc)