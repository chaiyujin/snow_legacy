cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(DepthPlayer)
# snow
add_subdirectory(../snow ext/snow/)
include_directories(${SNOW_INCLUDE_DIR})
# add pre-compiled ffmpeg 
include_directories("/home/chaiyujin/ffmpeg_build/include")
link_directories("/home/chaiyujin/ffmpeg_build/lib")

target_include_directories(DepthPlayer
    # link snow
    snow
    # link ffmpeg
    avcodec avutil avformat swresample swscale
    # link librealsense
    realsense2)
