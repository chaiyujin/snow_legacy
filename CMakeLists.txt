cmake_minimum_required(VERSION 3.0)
project(GLUtils)
# find package
find_package(assimp)
find_package(SDL2)
add_subdirectory(./glutils/third-party/glm-stable)

# manual add
include_directories(
    ./glutils/
    ./glutils/third-party/stb/
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
set(SNOW
    ./glutils/model/mesh.cpp
    ./glutils/model/model.cpp
    ./glutils/snow.cpp)
set(SOURCES main.cpp
            ${SNOW}
            ${THIRD_PARTY})

# set libraries
if (APPLE)
    # apple case
    find_library(OpenGL OpenGL)
    find_library(CoreFoundation CoreFoundation)
    set(LIBRARIES ${OpenGL} ${CoreFoundation})
endif ()
if (UNIX AND NOT APPLE)
    # linux case
    set(LIBRARIES GL X11 pthread Xrandr Xi dl)
endif ()
set(LIBRARIES ${LIBRARIES} SDL2 assimp glfw)
add_definitions(-O2)

# exec and link libaraies
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

