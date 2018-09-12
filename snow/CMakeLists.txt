cmake_minimum_required(VERSION 3.0)
project(snow VERSION 0.0.1 DESCRIPTION "A simple tool for OpenGL and GUI.")

set(SNOW_INCLUDE_DIR "")
set(SNOW_LIBRARIES_DIR "")

if (WIN32)
    # tell the package position
    set(ASSIMP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../third_party/assimp-3.1.1-win-binaries/include")
    set(ASSIMP_LIBRARIES_DIR "${PROJECT_SOURCE_DIR}/../third_party/assimp-3.1.1-win-binaries/lib64")

    set(SDL2_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../third_party/SDL2-2.0.8/include")
    set(SDL2_LIBRARIES_DIR "${PROJECT_SOURCE_DIR}/../third_party/SDL2-2.0.8/lib/x64")

    # set dll, cache
    set(SNOW_WIN32_DLLS
        "${PROJECT_SOURCE_DIR}/../third_party/assimp-3.1.1-win-binaries/bin64/assimp.dll"
        "${PROJECT_SOURCE_DIR}/../third_party/SDL2-2.0.8/lib/x64/SDL2.dll"
        CACHE INTERNAL "DIRS")

    # append include dirs
    set(SNOW_INCLUDE_DIR ${SNOW_INCLUDE_DIR} ${ASSIMP_INCLUDE_DIR} ${SDL2_INCLUDE_DIR})
    set(SNOW_LIBRARIES_DIR ${SNOW_LIBRARIES_DIR} ${ASSIMP_LIBRARIES_DIR} ${SDL2_LIBRARIES_DIR})

else ()
    # find package
    find_package(assimp)
    find_package(SDL2)
endif (WIN32)

macro (SNOW_COPY_DLL projectName)
    if (WIN32)
        foreach(THEDLL ${SNOW_WIN32_DLLS})
            message(STATUS "  |> Copy DLL: ${THEDLL}")
            add_custom_command(TARGET ${projectName} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                # source  # target
                ${THEDLL} $<TARGET_FILE_DIR:${projectName}>)
        endforeach(THEDLL ${SNOW_WIN32_DLLS})
    endif  (WIN32)
endmacro()

# manual add
set(THIRD_PARTY_INCLUDE_DIR
    ${PROJECT_SOURCE_DIR}/third-party/stb/
    ${PROJECT_SOURCE_DIR}/third-party/imgui/
    ${PROJECT_SOURCE_DIR}/third-party/glm-stable/  # glm
    ${PROJECT_SOURCE_DIR}/third-party/glad/include/)
set(SNOW_INCLUDE_DIR
    ${SNOW_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/
    ${THIRD_PARTY_INCLUDE_DIR})
include_directories(${SNOW_INCLUDE_DIR})
link_directories(${SNOW_LIBRARIES_DIR})
# cache dirs
set(SNOW_INCLUDE_DIR   ${SNOW_INCLUDE_DIR} CACHE INTERNAL "DIRS")
set(SNOW_LIBRARIES_DIR ${SNOW_LIBRARIES_DIR} CACHE INTERNAL "DIRS")

# set sources file
# build glad first (with out c++11)
if (UNIX)
    add_library(glad SHARED ${PROJECT_SOURCE_DIR}/third-party/glad/src/glad.c)  # glad
    target_link_libraries(glad dl)
else()
    add_library(glad STATIC ${PROJECT_SOURCE_DIR}/third-party/glad/src/glad.c)  # glad
endif (UNIX)

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
# WINDOWS
if (WIN32)
    set(LIBRARIES ${LIBRARIES} opengl32)
endif (WIN32)
# APPLE
if (APPLE)
    find_library(OpenGL OpenGL)
    find_library(CoreFoundation CoreFoundation)
    find_library(Cocoa Cocoa)
    set(LIBRARIES ${LIBRARIES} ${OpenGL} ${CoreFoundation} ${Cocoa})
    add_definitions( -DUSE_CLANG_COMPLETER )
    # append dialog.mm for macos
    set(SNOW_SOURCES ${SNOW_SOURCES} ${PROJECT_SOURCE_DIR}/gui/snow_dialog.mm)
endif ()
# LINUX
if (UNIX AND NOT APPLE)
    # linux case
    set(LIBRARIES ${LIBRARIES} GL X11 pthread Xrandr Xi)
endif ()

# shared libraries
set(LIBRARIES ${LIBRARIES} SDL2 assimp)

# exec and link libaraies
add_compile_options(-std=c++11 -O2)
add_library(${PROJECT_NAME} STATIC ${SNOW_SOURCES})
target_link_libraries(${PROJECT_NAME} glad ${LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER ${PROJECT_SOURCE_DIR}/snow.h
    OUTPUT_NAME ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_INCLUDE_DIR})
