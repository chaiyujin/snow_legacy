cmake_minimum_required(VERSION 3.0)

project(snow_examples)
add_subdirectory(snow)
include_directories(${SNOW_INCLUDE_DIR})

add_executable(example example/main.cpp)
target_link_libraries(example snow)
