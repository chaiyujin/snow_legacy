cmake_minimum_required(VERSION 3.0)
project(snow VERSION 0.0.1 DESCRIPTION "A simple tool for OpenGL and GUI.")
# find package
find_package(assimp)
find_package(SDL2)
# manual add
set(THIRD_PARTY_INCLUDE_DIR    
    ${PROJECT_SOURCE_DIR}/third-party/stb/
    ${PROJECT_SOURCE_DIR}/third-party/imgui/
    ${PROJECT_SOURCE_DIR}/third-party/glm-stable/  # glm
    ${PROJECT_SOURCE_DIR}/third-party/glad/include/)
set(SNOW_INCLUDE_DIR
    ${PROJECT_SOURCE_DIR}/
    ${THIRD_PARTY_INCLUDE_DIR}
    CACHE INTERNAL "DIRS")
include_directories(${SNOW_INCLUDE_DIR})
# set sources file
# build glad first (with out c++11)
add_library(glad STATIC
    ${PROJECT_SOURCE_DIR}/third-party/glad/src/glad.c)  # glad
set(THIRD_PARTY
    # imgui
    ${PROJECT_SOURCE_DIR}/third-party/imgui/imgui.cpp
    ${PROJECT_SOURCE_DIR}/third-party/imgui/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/third-party/imgui/imgui_demo.cpp
)
set(SNOW_SOURCES
    # main sources
    ${PROJECT_SOURCE_DIR}/model/snow_mesh.cpp
    ${PROJECT_SOURCE_DIR}/model/snow_model.cpp
    ${PROJECT_SOURCE_DIR}/gui/snow_app.cpp
    ${PROJECT_SOURCE_DIR}/gui/snow_imgui.cpp
    ${PROJECT_SOURCE_DIR}/gui/snow_window.cpp
    ${PROJECT_SOURCE_DIR}/gui/snow_dialog.cpp
    ${PROJECT_SOURCE_DIR}/tools/snow_arcball.cpp
    ${PROJECT_SOURCE_DIR}/tools/snow_camera.cpp)
# set sources
set(SNOW_SOURCES ${SNOW_SOURCES} ${THIRD_PARTY})

# set libraries
if (APPLE)
    # apple case
    find_library(OpenGL OpenGL)
    find_library(CoreFoundation CoreFoundation)
    find_library(Cocoa Cocoa)
    set(LIBRARIES ${OpenGL} ${CoreFoundation} ${Cocoa})
    add_definitions( -DUSE_CLANG_COMPLETER )
    set(SNOW_SOURCES ${SNOW_SOURCES}
                     ${PROJECT_SOURCE_DIR}/gui/snow_dialog.mm)
endif ()
if (UNIX AND NOT APPLE)
    # linux case
    set(LIBRARIES GL X11 pthread Xrandr Xi)
endif ()
set(LIBRARIES ${LIBRARIES} SDL2 assimp)

# exec and link libaraies
add_compile_options(-std=c++11 -O2)
add_library(${PROJECT_NAME} STATIC ${SNOW_SOURCES})
target_link_libraries(glad dl)
target_link_libraries(${PROJECT_NAME} glad ${LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER ${PROJECT_SOURCE_DIR}/snow.h
    OUTPUT_NAME ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_INCLUDE_DIR})
