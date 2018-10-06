cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(BilinearReconstruct)
# snow
set(SNOW_MODULE_OPENG on CACHE BOOL "enable opengl" FORCE)  # set module on
add_subdirectory(../../snow ext/snow/)
add_definitions(    ${SNOW_DEFINITIONS})
include_directories(${SNOW_INCLUDE_DIRS})
link_directories(   ${SNOW_LIBRARIES_DIRS})

find_package(Ceres)

add_executable(${PROJECT_NAME}
    ./main.cpp
    ./visualizer/window.cpp
    ./facedb/facedb.cpp
    ./facedb/tensor.cpp
    ./facedb/bilinear_model.cpp
    ./depth_source/realsense/rsutils.cpp
    ./depth_source/realsense/librealsense.cpp # no need to link realsense any more
)
target_link_libraries(${PROJECT_NAME}
    snow                    # link snow
    ceres
)

if (WIN32)
    SNOW_COPY_DLL(${PROJECT_NAME})
endif (WIN32)
