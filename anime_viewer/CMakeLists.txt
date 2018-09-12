cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(AnimeViewer)
add_subdirectory(../snow ext/snow/)
include_directories(${SNOW_INCLUDE_DIR})
link_directories(${SNOW_LIBRARIES_DIR})

set(SOURCES ./main.cpp
            ./biwi_obj.cpp
            ./window.cpp)
add_executable(AnimeViewer ${SOURCES})
target_link_libraries(AnimeViewer snow)

if (WIN32)
    SNOW_COPY_DLL(AnimeViewer)
endif (WIN32)

# === Python Binding ===
# 
# add_subdirectory(../pybind11 ext/pybind11/)
# pybind11_add_module(AnimeViewer ./pybind.cpp
#                                 ./biwi_obj.cpp
#                                 ./window.cpp)
# target_link_libraries(AnimeViewer PRIVATE "$<$<NOT:$<CONFIG:Debug>>:snow>")
