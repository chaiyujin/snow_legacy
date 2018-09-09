cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(DepthPlayer)
# snow
add_subdirectory(../snow ext/snow/)
include_directories(${SNOW_INCLUDE_DIR})
# add pre-compiled ffmpeg 
find_package(PkgConfig REQUIRED)
include_directories("/home/chaiyujin/ffmpeg_build/include")
set(ENV{PKG_CONFIG_PATH}  "/home/chaiyujin/ffmpeg_build/lib/pkgconfig")
pkg_search_module(PKG_AVCODEC    REQUIRED libavcodec) 
pkg_search_module(PKG_AVDEVICE   REQUIRED libavdevice)
pkg_search_module(PKG_AVFILTER   REQUIRED libavfilter)
pkg_search_module(PKG_AVFORMAT   REQUIRED libavformat)  
pkg_search_module(PKG_AVUTIL     REQUIRED libavutil)
pkg_search_module(PKG_POSTPROC   REQUIRED libpostproc)
pkg_search_module(PKG_SWRESAMPLE REQUIRED libswresample)  
pkg_search_module(PKG_SWSCALE    REQUIRED libswscale)

add_definitions(-O3)
add_executable(DepthPlayer
    ./main.cpp
    ./video/reader.cpp)
target_link_libraries(DepthPlayer
    # link snow
    snow
    # link ffmpeg
    ${PKG_AVCODEC_LDFLAGS}
    ${PKG_AVDEVICE_LDFLAGS}
    ${PKG_AVFILTER_LDFLAGS}
    ${PKG_AVFORMAT_LDFLAGS}
    ${PKG_AVUTIL_LDFLAGS}
    ${PKG_POSTPROC_LDFLAGS}
    ${PKG_SWRESAMPLE_LDFLAGS}
    ${PKG_SWSCALE_LDFLAGS}
    # link librealsense
    realsense2)
