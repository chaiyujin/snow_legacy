cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(DepthPlayer)
# snow
add_subdirectory(../snow ext/snow/)
include_directories(${SNOW_INCLUDE_DIR})
# add pre-compiled ffmpeg 

include(FindFFMpeg.cmake)

add_definitions(-O3)
add_executable(DepthPlayer
    ./main.cpp
    ./video/reader.cpp
    ./window/player.cpp)
target_link_libraries(DepthPlayer
    # link snow
    snow
    # link ffmpeg
    ${FFMPEG_LIBRARIES}
    # link librealsense
    realsense2)
