cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(DepthPlayer)
# snow
add_subdirectory(../snow ext/snow/)
include_directories(${SNOW_INCLUDE_DIR})
link_directories(${SNOW_LIBRARIES_DIR})
# add pre-compiled ffmpeg 

include(FFmpeg.cmake)
include_directories(${FFMPEG_INCLUDE_DIRS})

add_definitions(-O3)
add_executable(${PROJECT_NAME}
    ./main.cpp
    ./video/reader.cpp
    ./window/player.cpp)
target_link_libraries(${PROJECT_NAME}
    snow                    # link snow
    ${FFMPEG_LIBRARIES}     # link ffmpeg
    # realsense2            # link librealsense
)

if (WIN32)
    SNOW_COPY_DLL(${PROJECT_NAME})
    FFMPEG_COPY_DLL(${PROJECT_NAME})
endif (WIN32)
