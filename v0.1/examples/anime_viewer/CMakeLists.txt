cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(AnimeViewer)
set(SNOW_MODULE_FFMPEG on CACHE BOOL "enable ffmpeg" FORCE)  # set module on
add_subdirectory(../../snow ext/snow/)
include_directories(${SNOW_INCLUDE_DIRS})
link_directories(${SNOW_LIBRARIES_DIRS})
add_definitions(${SNOW_DEFINITIONS})

set(SOURCES 
    ./main.cpp
    ./gui/window.cpp
    ./obj/biwi_obj.cpp
    ./facedb/tensor.cpp
    ./facedb/facedb.cpp
    ./facedb/bilinear_model.cpp
    ./facedb/visualize.cpp
)
add_executable(AnimeViewer ${SOURCES})
target_link_libraries(AnimeViewer snow)

if (WIN32)
    SNOW_COPY_DLL(AnimeViewer)
endif (WIN32)

# === Python Binding ===
add_subdirectory(../../snow/third-party/pybind11 ext/pybind11/)
pybind11_add_module(anime_viewer11
    ./pybind.cpp
    ./gui/window.cpp
    ./obj/biwi_obj.cpp
    ./facedb/tensor.cpp
    ./facedb/facedb.cpp
    ./facedb/bilinear_model.cpp
    ./facedb/visualize.cpp
)
set_target_properties(snow PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(anime_viewer11 PRIVATE "$<$<NOT:$<CONFIG:Debug>>:snow>")
