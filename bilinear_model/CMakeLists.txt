cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)

project(Bilinear)
# snow
add_subdirectory(../snow ext/snow/)
include_directories(${SNOW_INCLUDE_DIRS})
link_directories(${SNOW_LIBRARIES_DIRS})
add_definitions(${SNOW_DEFINITIONS})

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )

add_executable(${PROJECT_NAME}
    ./main.cpp
    ./faceware/faceware.cpp
    ./faceware/show_model.cpp)
target_link_libraries(${PROJECT_NAME}
    snow                    # link snow
)

