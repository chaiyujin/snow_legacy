cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)
project(example)
add_subdirectory(snow)
add_subdirectory(pybind11)
include_directories(${SNOW_INCLUDE_DIR})

pybind11_add_module(example biwi_visualizer/main.cpp
                            biwi_visualizer/biwi_obj.cpp)
target_link_libraries(example PRIVATE "$<$<NOT:$<CONFIG:Debug>>:snow>")
