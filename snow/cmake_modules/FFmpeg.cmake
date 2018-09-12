# adapt from https://github.com/snikulov/cmake-modules

# Once done this will define
#    FFMPEG_FOUND                   - System has the all required components.
#    FFMPEG_INCLUDE_DIRS            - Include directory necessary for using the required components headers.
#    FFMPEG_LIBRARIES               - Link these to use the required ffmpeg components.
#    FFMPEG_DEFINITIONS             - Compiler switches required for using the required ffmpeg components.

#  ============== Prepare ==============
# - include dir: $HOME/ffmpeg_build/include
# - library dir: $HOME/ffmpeg_build/lib
# - binary  dir: $HOME/ffmpeg_build/bin
#  =====================================

if (NOT FFMPEG_INSTALL_PATH)
    if (UNIX)   # macos or Linux using Home
        set(FFMPEG_INSTALL_PATH "$ENV{HOME}/ffmpeg_build")
    else()      # windows using msys2_shell -mingw64
        set(FFMPEG_INSTALL_PATH "C:/msys64/home/admin/ffmpeg_build")
    endif()
endif ()

# set components to find
if (NOT FFMPEG_FIND_COMPONENTS)
    set(FFMPEG_FIND_COMPONENTS AVCODEC AVDEVICE AVFORMAT AVUTIL POSTPROC SWSCALE SWRESAMPLE)
endif ()


#  ============== Find ==============
# - ${_component}_FOUND
# - ${_component}_LIBRARIES
# - ${_component}_INCLUDE_DIRS
#  ==================================

macro(SET_COMPONENT_FOUND _component )
    if (${_component}_LIBRARIES AND ${_component}_INCLUDE_DIRS)
        set(${_component}_FOUND TRUE)
    endif ()
endmacro()

macro(FIND_COMPONENT _component _pkgconfig _library _header)
    
    if (UNIX)
        # use pkg config to find ffmpeg build from shell
        find_package(PkgConfig REQUIRED)
        set(ENV{PKG_CONFIG_PATH} "${FFMPEG_INSTALL_PATH}/lib/pkgconfig")
        pkg_check_modules(PC_${_component} ${_pkgconfig})

        # find from path
        find_path(${_component}_INCLUDE_DIRS       ${_header}   HINTS "${FFMPEG_INSTALL_PATH}/include/" )
        find_library(${_component}_LIBRARIES NAMES ${_library}  HINTS "${FFMPEG_INSTALL_PATH}/lib/" )
        
        # process ldflags
        if (NOT APPLE)
            set(${_component}_LIBRARIES ${_component}_LIBRARIES ${PC_${_component}_LDFLAGS})
        endif (NOT APPLE)

        set(${_component}_DEFINITIONS   ${PC_${_component}_CFLAGS_OTHER} CACHE STRING "The ${_component} CFLAGS.")
        set(${_component}_VERSION       ${PC_${_component}_VERSION}      CACHE STRING "The ${_component} version number.")

    else()
        # in windows
        find_path(${_component}_INCLUDE_DIRS       ${_header}  HINTS "${FFMPEG_INSTALL_PATH}/include/" )
        find_library(${_component}_LIBRARIES NAMES ${_library} HINTS "${FFMPEG_INSTALL_PATH}/lib/")
        include_directories(${_component}_INCLUDE_DIRS)
    endif (UNIX)

    set_component_found(${_component})
    mark_as_advanced(
        ${_component}_INCLUDE_DIRS
        ${_component}_LIBRARIES
        ${_component}_DEFINITIONS
        ${_component}_VERSION)

endmacro()

macro (FFMPEG_COPY_DLL projectName)
    if (WIN32)
        foreach(THEDLL ${FFMPEG_WIN32_DLLS})
            message(STATUS "  |> Copy DLL: ${THEDLL}")
            add_custom_command(TARGET ${projectName} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${THEDLL} $<TARGET_FILE_DIR:${projectName}>) # source  # target
        endforeach(THEDLL ${SNOW_WIN32_DLLS})
    endif  (WIN32)
endmacro()

# ====== begin to find ======
if (NOT FFMPEG_FOUND)
    set(FFMPEG_INCLUDE_DIRS "")
    set(FFMPEG_LIBRARIES    "")
    set(FFMPEG_DEFINITIONS  "")
    set(FFMPEG_WIN32_DLLS   "")

    # Check for all possible component.
    find_component(AVCODEC    libavcodec    avcodec    libavcodec/avcodec.h)
    find_component(AVFORMAT   libavformat   avformat   libavformat/avformat.h)
    find_component(AVDEVICE   libavdevice   avdevice   libavdevice/avdevice.h)
    find_component(AVUTIL     libavutil     avutil     libavutil/avutil.h)
    find_component(AVFILTER   libavfilter   avfilter   libavfilter/avfilter.h)
    find_component(SWSCALE    libswscale    swscale    libswscale/swscale.h)
    find_component(POSTPROC   libpostproc   postproc   libpostproc/postprocess.h)
    find_component(SWRESAMPLE libswresample swresample libswresample/swresample.h)

    # Check if the required components were found and add their stuff to the FFMPEG_* vars.
    foreach (_component ${FFMPEG_FIND_COMPONENTS})
        if (${_component}_FOUND)
            # message(STATUS "Required component ${_component} present.")
            set(FFMPEG_LIBRARIES   ${FFMPEG_LIBRARIES}   ${${_component}_LIBRARIES})
            set(FFMPEG_DEFINITIONS ${FFMPEG_DEFINITIONS} ${${_component}_DEFINITIONS})
            list(APPEND FFMPEG_INCLUDE_DIRS ${${_component}_INCLUDE_DIRS})
        else ()
            message(STATUS "FFmpeg component ${_component} missing.")
        endif ()
    endforeach ()

    # Build the include path with duplicates removed.
    if (FFMPEG_INCLUDE_DIRS)
        list(REMOVE_DUPLICATES FFMPEG_INCLUDE_DIRS)
    endif ()

    # cache the vars.
    set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS} CACHE STRING "The FFmpeg include directories." FORCE)
    set(FFMPEG_LIBRARIES    ${FFMPEG_LIBRARIES}    CACHE STRING "The FFmpeg libraries." FORCE)
    set(FFMPEG_DEFINITIONS  ${FFMPEG_DEFINITIONS}  CACHE STRING "The FFmpeg cflags." FORCE)
    
    if (WIN32)
        file(GLOB FFMPEG_WIN32_DLLS "${FFMPEG_INSTALL_PATH}/bin/*.dll" )
    endif (WIN32)

    mark_as_advanced(FFMPEG_INCLUDE_DIRS
                     FFMPEG_LIBRARIES
                     FFMPEG_DEFINITIONS
                     FFMPEG_WIN32_DLLS)

endif ()
