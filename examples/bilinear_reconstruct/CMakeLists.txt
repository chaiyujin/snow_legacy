cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(BilinearReconstruct)
# snow
set(SNOW_MODULE_OPENG on CACHE BOOL "enable opengl" FORCE)  # set module on
add_subdirectory(../../snow ext/snow/)
add_definitions(    ${SNOW_DEFINITIONS})
include_directories(${SNOW_INCLUDE_DIRS})
link_directories(   ${SNOW_LIBRARIES_DIRS})

add_executable(${PROJECT_NAME}
    ./main.cpp
    ./visualizer/show.cpp
    ./depth_source/realsense/rsutils.cpp)
target_link_libraries(${PROJECT_NAME}
    snow                    # link snow
    realsense2              # link librealsense
)

