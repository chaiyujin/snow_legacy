cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(VideoPlayer)
# snow
set(SNOW_MODULE_FFMPEG on CACHE BOOL "enable ffmpeg" FORCE)  # set module on
add_subdirectory(../../snow ext/snow/)
add_definitions(    ${SNOW_DEFINITIONS})
include_directories(${SNOW_INCLUDE_DIRS})
link_directories(   ${SNOW_LIBRARIES_DIRS})

add_executable(${PROJECT_NAME}
    ./main.cpp
    ./window/player.cpp)
target_link_libraries(${PROJECT_NAME}
    snow                    # link snow
    # realsense2            # link librealsense
)

if (WIN32)
    SNOW_COPY_DLL(${PROJECT_NAME})
endif (WIN32)
