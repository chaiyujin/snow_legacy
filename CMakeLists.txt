cmake_minimum_required(VERSION 3.0)

project(biwi_vis)
add_subdirectory(snow)
include_directories(${SNOW_INCLUDE_DIR})

add_compile_options(-std=c++11 -O2)
add_executable(biwi_vis biwi_visualizer/main.cpp
                        biwi_visualizer/biwi_obj.cpp)
target_link_libraries(biwi_vis snow)
