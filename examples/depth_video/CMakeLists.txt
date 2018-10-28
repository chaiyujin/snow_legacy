cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(depth)
# snow
set(SNOW_MODULE_FFMPEG on CACHE BOOL "enable ffmpeg" FORCE)  # set module on
add_subdirectory(../../snow ext/snow/)
include_directories(${SNOW_INCLUDE_DIRS})
link_directories(${SNOW_LIBRARIES_DIRS})
add_definitions(${SNOW_DEFINITIONS})

add_executable(${PROJECT_NAME}
    ./main.cpp
    ./process.cpp)
target_link_libraries(${PROJECT_NAME} snow)

# === Python Binding ===
add_subdirectory(../../snow/third-party/pybind11 ext/pybind11/)
pybind11_add_module(depth11
    ./pybind.cpp
    ./process.cpp)
set_target_properties(snow PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(depth11 PRIVATE "$<$<NOT:$<CONFIG:Debug>>:snow>")
