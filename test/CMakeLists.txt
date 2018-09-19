cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(snow_test)
# snow
set(SNOW_AVX            on CACHE BOOL "enable avx"       FORCE)
set(SNOW_RELEASE        on CACHE BOOL "enable release"   FORCE)
set(SNOW_MODULE_OPENGL  on CACHE BOOL "enable opengl"    FORCE)  # set module on
set(SNOW_MODULE_FFMPEG  on CACHE BOOL "enable ffmpeg"    FORCE)  # set module on
add_subdirectory(../snow ext/snow/)
add_definitions(    ${SNOW_DEFINITIONS})
link_directories(   ${SNOW_LIBRARIES_DIRS})
include_directories(${SNOW_INCLUDE_DIRS})

add_executable(test_simd test_simd.cpp)
add_executable(test_memory test_memory.cpp)
target_link_libraries(test_simd snow)
target_link_libraries(test_memory snow)

if (WIN32)
    SNOW_COPY_DLL(${PROJECT_NAME})
endif (WIN32)
