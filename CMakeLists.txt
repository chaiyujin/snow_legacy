cmake_minimum_required(VERSION 3.0)
project(GLUtils)
# find package
find_package(SDL2)

# manual add
include_directories(
    ./glutils/third-party/imgui/
    ./glutils/third-party/glad/include/
)

# set sources file
set(THIRD_PARTY
    ./glutils/third-party/glad/src/glad.c  # glad
    ./glutils/third-party/imgui/imgui.cpp
    ./glutils/third-party/imgui/imgui_draw.cpp
    ./glutils/third-party/imgui/imgui_demo.cpp
    ./glutils/third-party/imgui/imgui_impl_opengl3.cpp
    ./glutils/third-party/imgui/imgui_impl_sdl.cpp
)
set(SOURCES main.cpp ${THIRD_PARTY})

# set libraries
if (APPLE)
    # apple case
    find_library(OpenGL OpenGL)
    find_library(CoreFoundation CoreFoundation)
    set(LIBRARIES SDL2 ${OpenGL} ${CoreFoundation})
endif(APPLE)
if (LINUX)
    # linux case
    set(LIBRARIES GL SDL2 opengl32)
endif (LINUX)

# exec and link libaraies
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

