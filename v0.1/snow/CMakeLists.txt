cmake_minimum_required(VERSION 3.0)
project(snow VERSION 0.0.2 DESCRIPTION "A simple tool for OpenGL, Gui and FFmpeg.")
# at least c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD 11 PARENT_SCOPE)

OPTION(SNOW_AVX              "AVX functions" off)
OPTION(SNOW_MODULE_OPENGL    "Build with module opengl" on)
OPTION(SNOW_MODULE_FFMPEG    "Build with module ffmpeg" off)
OPTION(SNOW_MODULE_FP        "Build with module functional programming" off)

# ====================================
# ============== Begin ===============
# ====================================
set(SNOW_SOURCES        "")
set(SNOW_INCLUDE_DIRS   "${PROJECT_SOURCE_DIR}")
set(SNOW_LIBRARIES_DIRS "")
set(SNOW_LIBRARIES      "")
set(SNOW_WIN32_DLLS     "")
set(SNOW_DEFINITIONS    "")

# add defitions
if (NOT CMAKE_BUILD_TYPE MATCHES "")
    string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
endif (NOT CMAKE_BUILD_TYPE MATCHES "")
if (CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    message(STATUS "snow debug")
elseif(CMAKE_BUILD_TYPE MATCHES "[Rr][Ee][Ll][Ee][Aa][Ss][Ee]")
    message(STATUS "snow release")
    list(APPEND SNOW_DEFINITIONS    "-DNDEBUG")
    if (NOT MSVC)
        list(APPEND SNOW_DEFINITIONS    "-O3")
    endif()
else()
    message(SEND_ERROR "unknow CMAKE_BUILD_TYPE")
endif ()

if (SNOW_AVX)
    list(APPEND SNOW_DEFINITIONS    "-mavx")
    list(APPEND SNOW_DEFINITIONS    "-DSNOW_AVX")
endif (SNOW_AVX)

# warnings
if (MSVC)
    list(APPEND SNOW_DEFINITIONS    "-D_CRT_SECURE_NO_WARNINGS")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -pedantic")
endif (MSVC)


# ====================================
# =========== Module Core ============
# ====================================
set(SNOW_CORE_SOURCES        "${PROJECT_SOURCE_DIR}/core/data/snow_image.cpp"
                             "${PROJECT_SOURCE_DIR}/core/tools/snow_log.cpp")
set(SNOW_CORE_INCLUDE_DIRS   "${PROJECT_SOURCE_DIR}/third-party/stb/")
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

# spdlog
add_subdirectory("${PROJECT_SOURCE_DIR}/third-party/spdlog")
list(APPEND SNOW_INCLUDE_DIRS   spdlog)
list(APPEND SNOW_LIBRARIES      spdlog)


# append to snow
list(APPEND SNOW_SOURCES        ${SNOW_CORE_SOURCES})
list(APPEND SNOW_INCLUDE_DIRS   ${SNOW_CORE_INCLUDE_DIRS})
list(APPEND SNOW_LIBRARIES_DIRS ${SNOW_CORE_LIBRARIES_DIRS})
list(APPEND SNOW_LIBRARIES      ${SNOW_CORE_LIBRARIES})


# ====================================
# ============ Module FP =============
# ====================================
if (SNOW_MODULE_FP)
    # c++ 14
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD 14 PARENT_SCOPE)
    list(APPEND SNOW_DEFINITIONS "-DSNOW_MODULE_LIBFP")
    list(APPEND SNOW_INCLUDE_DIRS ${SNOW_FP_INCLUDE_DIRS})
endif (SNOW_MODULE_FP)


# ====================================
# ========== Module OpenGL ===========
# ====================================
if (SNOW_MODULE_OPENGL)
    list(APPEND SNOW_DEFINITIONS "-DSNOW_MODULE_OPENGL")
    set(SNOW_OPENGL_SOURCES
        "${PROJECT_SOURCE_DIR}/opengl/tools/snow_arcball.cpp"
        "${PROJECT_SOURCE_DIR}/opengl/tools/snow_camera.cpp"
        "${PROJECT_SOURCE_DIR}/opengl/gui/snow_app.cpp"
        "${PROJECT_SOURCE_DIR}/opengl/gui/snow_text.cpp"
        "${PROJECT_SOURCE_DIR}/opengl/gui/snow_imgui.cpp"
        "${PROJECT_SOURCE_DIR}/opengl/gui/snow_window.cpp"
        "${PROJECT_SOURCE_DIR}/opengl/gui/snow_dialog.cpp"
        "${PROJECT_SOURCE_DIR}/opengl/model/snow_mesh.cpp"
        "${PROJECT_SOURCE_DIR}/opengl/model/snow_model.cpp"
        # third-party ==== ImGui ====
        "${PROJECT_SOURCE_DIR}/third-party/imgui/imgui.cpp"
        "${PROJECT_SOURCE_DIR}/third-party/imgui/imgui_draw.cpp"
        "${PROJECT_SOURCE_DIR}/third-party/imgui/imgui_demo.cpp")
    set(SNOW_OPENGL_INCLUDE_DIRS
        # third-party
        "${PROJECT_SOURCE_DIR}/third-party/imgui/"
        "${PROJECT_SOURCE_DIR}/third-party/glad/include/")
    set(SNOW_OPENGL_LIBRARIES_DIRS "")
    set(SNOW_OPENGL_LIBRARIES      "")

    # === GLAD ===
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

    # === freetype ===
    if (WIN32)
        list(APPEND SNOW_OPENGL_INCLUDE_DIRS    "${PROJECT_SOURCE_DIR}/../third_party/freetype/include")
        list(APPEND SNOW_OPENGL_LIBRARIES_DIRS  "${PROJECT_SOURCE_DIR}/../third_party/freetype/win64")
        list(APPEND SNOW_OPENGL_LIBRARIES       freetype)
        list(APPEND SNOW_WIN32_DLLS             "${PROJECT_SOURCE_DIR}/../third_party/freetype/win64/freetype.dll")
    else ()
        find_package(Freetype REQUIRED)
        list(APPEND SNOW_OPENGL_INCLUDE_DIRS    "${FREETYPE_INCLUDE_DIRS}")
        list(APPEND SNOW_OPENGL_LIBRARIES       "${FREETYPE_LIBRARIES}")
    endif ()

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
        list(APPEND SNOW_OPENGL_SOURCES "${PROJECT_SOURCE_DIR}/opengl/gui/snow_dialog.mm")
    elseif (UNIX) # LINUX
        list(APPEND SNOW_OPENGL_LIBRARIES GL X11 pthread Xrandr Xi dl)
    endif ()

    list(APPEND SNOW_SOURCES        ${SNOW_OPENGL_SOURCES})
    list(APPEND SNOW_INCLUDE_DIRS   ${SNOW_OPENGL_INCLUDE_DIRS})
    list(APPEND SNOW_LIBRARIES_DIRS ${SNOW_OPENGL_LIBRARIES_DIRS})
    list(APPEND SNOW_LIBRARIES      ${SNOW_OPENGL_LIBRARIES})
endif (SNOW_MODULE_OPENGL)


# ====================================
# ========== Module Media ============
# ====================================
# simple media
list(APPEND SNOW_SOURCES
        "${PROJECT_SOURCE_DIR}/media/basic/snow_wav.cpp")
# ffmpeg
if (SNOW_MODULE_FFMPEG)
    list(APPEND SNOW_DEFINITIONS "-DSNOW_MODULE_FFMPEG")
    
    include(cmake_modules/FFmpeg.cmake)
    set(SNOW_FFMPEG_SOURCES
        "${PROJECT_SOURCE_DIR}/media/ffmpeg/snow_reader.cpp"
        "${PROJECT_SOURCE_DIR}/media/ffmpeg/snow_writer.cpp"
        "${PROJECT_SOURCE_DIR}/media/ffmpeg/ffmpeg_functions.cpp")

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


# === exec and link libaraies ===
add_definitions      (${SNOW_DEFINITIONS})
include_directories  (${SNOW_INCLUDE_DIRS}                  )
link_directories     (${SNOW_LIBRARIES_DIRS}                )
add_library          (${PROJECT_NAME} STATIC ${SNOW_SOURCES})
target_link_libraries(${PROJECT_NAME} ${SNOW_LIBRARIES}     )
