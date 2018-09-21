cmake_minimum_required(VERSION 3.0)
project(snow VERSION 0.0.2 DESCRIPTION "A simple tool for OpenGL, Gui and FFmpeg.")
set(CMAKE_CXX_STANDARD 11)

OPTION(SNOW_AVX              "AVX functions" on)
OPTION(SNOW_RELEASE          "-O3 switch"    on)

OPTION(SNOW_MODULE_OPENGL    "Build with module opengl" on)
OPTION(SNOW_MODULE_FFMPEG    "Build with module ffmpeg" off)
OPTION(SNOW_MODULE_FP        "Build with module functional programming" off)

if (SNOW_MODULE_FP)
    set(CMAKE_CXX_STANDARD 14)
endif (SNOW_MODULE_FP)

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
set(SNOW_CORE_SOURCES   "")
set(SNOW_CORE_INCLUDE_DIRS
    # third-party
    "${PROJECT_SOURCE_DIR}/third-party/stb/")
set(SNOW_CORE_LIBRARIES_DIRS "")
set(SNOW_CORE_LIBRARIES      "")
# glm
if (SNOW_AVX)
    set(GLM_TEST_ENABLE_SIMD_AVX on CACHE BOOL "enable glm avx" FORCE)
endif (SNOW_AVX)
add_subdirectory("${PROJECT_SOURCE_DIR}/third-party/glm-stable")
list(APPEND SNOW_INCLUDE_DIRS   ${GLM_INCLUDE_DIRS})
list(APPEND SNOW_LIBRARIES      glm)
# eigen3
find_package(Eigen3 3.3 REQUIRED)
list(APPEND SNOW_LIBRARIES      Eigen3::Eigen)

list(APPEND SNOW_SOURCES        ${SNOW_CORE_SOURCES})
list(APPEND SNOW_INCLUDE_DIRS   ${SNOW_CORE_INCLUDE_DIRS})
list(APPEND SNOW_LIBRARIES_DIRS ${SNOW_CORE_LIBRARIES_DIRS})
list(APPEND SNOW_LIBRARIES      ${SNOW_CORE_LIBRARIES})
if (SNOW_RELEASE)
    list(APPEND SNOW_DEFINITIONS    "-O3")
    list(APPEND SNOW_DEFINITIONS    "-DNDEBUG")
else()
    list(APPEND SNOW_DEFINITIONS    "-D__DEBUG__")
endif (SNOW_RELEASE)
if (SNOW_AVX)
    list(APPEND SNOW_DEFINITIONS    "-mavx")
    list(APPEND SNOW_DEFINITIONS    "-DSNOW_AVX")
endif (SNOW_AVX)

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
        list(APPEND SNOW_OPENGL_LIBRARIES GL X11 pthread Xrandr Xi dl)
    endif ()

    list(APPEND SNOW_SOURCES        ${SNOW_OPENGL_SOURCES})
    list(APPEND SNOW_INCLUDE_DIRS   ${SNOW_OPENGL_INCLUDE_DIRS})
    list(APPEND SNOW_LIBRARIES_DIRS ${SNOW_OPENGL_LIBRARIES_DIRS})
    list(APPEND SNOW_LIBRARIES      ${SNOW_OPENGL_LIBRARIES})
endif (SNOW_MODULE_OPENGL)

# ====================================
# ========== Module FFmpeg ===========
# ====================================

if (SNOW_MODULE_FFMPEG)
    list(APPEND SNOW_DEFINITIONS "-DSNOW_MODULE_FFMPEG")
    
    include(cmake_modules/FFmpeg.cmake)
    set(SNOW_FFMPEG_SOURCES "${PROJECT_SOURCE_DIR}/media/snow_reader.cpp")

    list(APPEND SNOW_SOURCES            "${SNOW_FFMPEG_SOURCES}")
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

# exec and link libaraies
add_definitions      (${SNOW_DEFINITIONS})
include_directories  (${SNOW_INCLUDE_DIRS}                  )
link_directories     (${SNOW_LIBRARIES_DIRS}                )
add_library          (${PROJECT_NAME} SHARED ${SNOW_SOURCES})
target_link_libraries(${PROJECT_NAME} ${SNOW_LIBRARIES}     )
