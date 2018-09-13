cmake_minimum_required(VERSION 3.0)
project(snow VERSION 0.0.2 DESCRIPTION "A simple tool for OpenGL, Gui and FFmpeg.")

OPTION(SNOW_MODULE_OPENGL    "Build with module opengl" on)
OPTION(SNOW_MODULE_FFMPEG    "Build with module ffmpeg" off)

# ============== Begin ===============
set(SNOW_SOURCES        "")
set(SNOW_INCLUDE_DIRS   "${PROJECT_SOURCE_DIR}")
set(SNOW_LIBRARIES_DIRS "")
set(SNOW_LIBRARIES      "")
set(SNOW_WIN32_DLLS     "")
set(SNOW_DEFINITIONS    "")

# ====================================
# =========== Module Core ============
# ====================================
set(SNOW_CORE_SOURCES       "")
set(SNOW_CORE_INCLUDE_DIRS
    # third-party
    "${PROJECT_SOURCE_DIR}/third-party/stb/"
    "${PROJECT_SOURCE_DIR}/third-party/glm-stable/")
set(SNOW_CORE_LIBRARIES_DIRS "")
set(SNOW_CORE_LIBRARIES     "")

list(APPEND SNOW_SOURCES       ${SNOW_CORE_SOURCES})
list(APPEND SNOW_INCLUDE_DIRS   ${SNOW_CORE_INCLUDE_DIRS})
list(APPEND SNOW_LIBRARIES_DIRS ${SNOW_CORE_LIBRARIES_DIRS})
list(APPEND SNOW_LIBRARIES     ${SNOW_CORE_LIBRARIES})

# ====================================
# ========== Module OpenGL ===========
# ====================================
if (SNOW_MODULE_OPENGL)
    list(APPEND SNOW_DEFINITIONS "-DSNOW_MODULE_OPENGL")
    set(SNOW_OPENGL_SOURCES
        "${PROJECT_SOURCE_DIR}/gui/snow_app.cpp"
        "${PROJECT_SOURCE_DIR}/gui/snow_imgui.cpp"
        "${PROJECT_SOURCE_DIR}/gui/snow_window.cpp"
        "${PROJECT_SOURCE_DIR}/gui/snow_dialog.cpp"
        "${PROJECT_SOURCE_DIR}/model/snow_mesh.cpp"
        "${PROJECT_SOURCE_DIR}/model/snow_model.cpp"
        "${PROJECT_SOURCE_DIR}/tools/snow_arcball.cpp"
        "${PROJECT_SOURCE_DIR}/tools/snow_camera.cpp"
        # third-party ==== ImGui ====
        "${PROJECT_SOURCE_DIR}/third-party/imgui/imgui.cpp"
        "${PROJECT_SOURCE_DIR}/third-party/imgui/imgui_draw.cpp"
        "${PROJECT_SOURCE_DIR}/third-party/imgui/imgui_demo.cpp")
    set(SNOW_OPENGL_INCLUDE_DIRS
        # third-party
        "${PROJECT_SOURCE_DIR}/third-party/imgui/"
        "${PROJECT_SOURCE_DIR}/third-party/glad/include/")
    set(SNOW_OPENGL_LIBRARIES_DIRS "")
    set(SNOW_OPENGL_LIBRARIES     "")

    # === GLAD ===
    # add_library(OPENGL_GLAD STATIC "${PROJECT_SOURCE_DIR}/third-party/glad/src/glad.c")
    # list(APPEND SNOW_OPENGL_LIBRARIES OPENGL_GLAD)  # append glad
    list(APPEND SNOW_OPENGL_SOURCES "${PROJECT_SOURCE_DIR}/third-party/glad/src/glad.c")

    # === SDL2 ===
    include(cmake_modules/SDL2.cmake)
    list(APPEND SNOW_OPENGL_INCLUDE_DIRS    "${SDL2_INCLUDE_DIRS}")
    list(APPEND SNOW_OPENGL_LIBRARIES_DIRS  "${SDL2_LIBRARIES_DIRS}")
    list(APPEND SNOW_WIN32_DLLS             "${SDL2_WIN32_DLLS}")
    list(APPEND SNOW_OPENGL_LIBRARIES       SDL2)

    # === assimp ===
    include(cmake_modules/assimp.cmake)
    list(APPEND SNOW_OPENGL_INCLUDE_DIRS    "${ASSIMP_INCLUDE_DIRS}")
    list(APPEND SNOW_OPENGL_LIBRARIES_DIRS  "${ASSIMP_LIBRARIES_DIRS}")
    list(APPEND SNOW_WIN32_DLLS             "${ASSIMP_WIN32_DLLS}")
    list(APPEND SNOW_OPENGL_LIBRARIES       assimp)

    # === GL ====
    if (WIN32)
        list(APPEND SNOW_OPENGL_LIBRARIES opengl32)
    elseif (APPLE)
        find_library(Cocoa Cocoa)
        find_library(OpenGL OpenGL)
        find_library(CoreFoundation CoreFoundation)
        list(APPEND SNOW_OPENGL_LIBRARIES "${Cocoa}" "${OpenGL}" "${CoreFoundation}")
        # append dialog.mm for macos
        add_definitions( -DUSE_CLANG_COMPLETER )
        list(APPEND SNOW_OPENGL_SOURCES "${PROJECT_SOURCE_DIR}/gui/snow_dialog.mm")
    elseif (UNIX) # LINUX
        list(APPEND SNOW_OPENGL_LIBRARIES GL X11 pthread Xrandr Xi)
    endif ()

    list(APPEND SNOW_SOURCES        ${SNOW_OPENGL_SOURCES})
    list(APPEND SNOW_INCLUDE_DIRS   ${SNOW_OPENGL_INCLUDE_DIRS})
    list(APPEND SNOW_LIBRARIES_DIRS ${SNOW_OPENGL_LIBRARIES_DIRS})
    list(APPEND SNOW_LIBRARIES      ${SNOW_OPENGL_LIBRARIES})
endif (SNOW_MODULE_OPENGL)

if (SNOW_MODULE_FFMPEG)
    include(cmake_modules/FFmpeg.cmake)
    list(APPEND SNOW_INCLUDE_DIRS       "${FFMPEG_INCLUDE_DIRS}")
    list(APPEND SNOWL_LIBRARIES_DIRS    "${FFMPEG_LIBRARIES_DIRS}")
    list(APPEND SNOW_WIN32_DLLS         "${FFMPEG_WIN32_DLLS}")
    list(APPEND SNOW_LIBRARIES          "${FFMPEG_LIBRARIES}")
    
endif (SNOW_MODULE_FFMPEG)

# === CACHE ===
set(SNOW_SOURCES        "${SNOW_SOURCES}"        CACHE INTERNAL "snow sources")
set(SNOW_INCLUDE_DIRS   "${SNOW_INCLUDE_DIRS}"   CACHE INTERNAL "snow include directories")
set(SNOW_LIBRARIES_DIRS "${SNOW_LIBRARIES_DIRS}" CACHE INTERNAL "snow link directories")
set(SNOW_LIBRARIES      "${SNOW_LIBRARIES}"      CACHE INTERNAL "snow libraries")
set(SNOW_WIN32_DLLS     "${SNOW_WIN32_DLLS}"     CACHE INTERNAL "snow win32 dlls")
set(SNOW_DEFINITIONS    "${SNOW_DEFINITIONS}"    CACHE INTERNAL "snow definitions")

# === macro to copy dlls ===
macro (SNOW_COPY_DLL projectName)
    foreach(THEDLL ${SNOW_WIN32_DLLS})
        message(STATUS "  |> Copy DLL: ${THEDLL}")
        add_custom_command(TARGET ${projectName} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            # source  # target
            ${THEDLL} $<TARGET_FILE_DIR:${projectName}>)
    endforeach(THEDLL ${SNOW_WIN32_DLLS})
endmacro()

list(APPEND CMAKE_CXX_FLAGS "-std=c++11" "-O3")
# exec and link libaraies
add_definitions      (${SNOW_DEFINITIONS})
include_directories  (${SNOW_INCLUDE_DIRS}                  )
link_directories     (${SNOW_LIBRARIES_DIRS}                )
add_library          (${PROJECT_NAME} STATIC ${SNOW_SOURCES})
target_link_libraries(${PROJECT_NAME} ${SNOW_LIBRARIES}     )
