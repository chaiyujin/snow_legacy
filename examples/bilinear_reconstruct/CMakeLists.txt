cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(BilinearReconstruct)
# snow
set(SNOW_MODULE_OPENG on CACHE BOOL "enable opengl" FORCE)  # set module on
add_subdirectory(../../snow ext/snow/)
add_definitions(    ${SNOW_DEFINITIONS})
include_directories(${SNOW_INCLUDE_DIRS})
link_directories(   ${SNOW_LIBRARIES_DIRS})

if (WIN32)
    add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)
    include_directories(../../third_party/glog)
    include_directories(../../third_party/ceres/include)
    link_directories(../../third_party/ceres/lib/release)
    set(SNOW_WIN32_DLLS "${SNOW_WIN32_DLLS}" "D:/Projects/snow/third_party/ceres/lib/release/ceres.dll" CACHE INTERNAL "snow win32 dlls")
else()
    find_package(Ceres)
endif (WIN32)


add_executable(${PROJECT_NAME}
    ./main.cpp
    ./visualizer/window.cpp
    ./facedb/facedb.cpp
    ./facedb/tensor.cpp
    ./facedb/bilinear_model.cpp
    ./depth_source/realsense/rsutils.cpp
    ./depth_source/realsense/librealsense.cpp # no need to link realsense any more
    ./solver/frames_solver.cpp
    ./solver/video_solver.cpp
)
target_link_libraries(${PROJECT_NAME}
    snow                    # link snow
    ceres
)

if (WIN32)
    SNOW_COPY_DLL(${PROJECT_NAME})
endif (WIN32)
