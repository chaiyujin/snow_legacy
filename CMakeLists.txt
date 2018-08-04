cmake_minimum_required(VERSION 3.0)
project(GLUtils)
# find package
find_package(assimp)
find_package(SDL2)

# manual add
include_directories(
    ${PROJECT_SOURCE_DIR}/glutils/
    ${PROJECT_SOURCE_DIR}/glutils/snow/
    ${PROJECT_SOURCE_DIR}/glutils/third-party/stb/
    ${PROJECT_SOURCE_DIR}/glutils/third-party/imgui/
    ${PROJECT_SOURCE_DIR}/glutils/third-party/glm-stable/  # glm
    ${PROJECT_SOURCE_DIR}/glutils/third-party/glad/include/
)
# set sources file
# build glad first (with out c++11)
add_library(GLAD STATIC
    ${PROJECT_SOURCE_DIR}/glutils/third-party/glad/src/glad.c)  # glad
set(THIRD_PARTY
    # imgui
    ${PROJECT_SOURCE_DIR}/glutils/third-party/imgui/imgui.cpp
    ${PROJECT_SOURCE_DIR}/glutils/third-party/imgui/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/glutils/third-party/imgui/imgui_demo.cpp
    ${PROJECT_SOURCE_DIR}/glutils/third-party/imgui/imgui_impl_opengl3.cpp
    ${PROJECT_SOURCE_DIR}/glutils/third-party/imgui/imgui_impl_sdl.cpp
)
set(SNOW
    # main sources
    ${PROJECT_SOURCE_DIR}/glutils/model/mesh.cpp
    ${PROJECT_SOURCE_DIR}/glutils/model/model.cpp
    ${PROJECT_SOURCE_DIR}/glutils/snow/snow.cpp
    ${PROJECT_SOURCE_DIR}/glutils/snow/snow_app.cpp
    ${PROJECT_SOURCE_DIR}/glutils/snow/snow_window.cpp)
# set sources
set(SOURCES main.cpp ${SNOW} ${THIRD_PARTY})

# set libraries
if (APPLE)
    # apple case
    find_library(OpenGL OpenGL)
    find_library(CoreFoundation CoreFoundation)
    set(LIBRARIES ${OpenGL} ${CoreFoundation})
endif ()
if (UNIX AND NOT APPLE)
    # linux case
    set(LIBRARIES GL X11 pthread Xrandr Xi)
endif ()
set(LIBRARIES ${LIBRARIES} SDL2 assimp)

# exec and link libaraies
add_compile_options(-std=c++11 -O2)
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(GLAD dl)
target_link_libraries(${PROJECT_NAME} GLAD ${LIBRARIES})
