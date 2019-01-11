cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
PROJECT(snow)

# v=== prepare necessary macros ===v
# # build type
IF(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    SET(CMAKE_BUILD_TYPE Debug)
ENDIF(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")

# # build platform
IF(NOT DEFINED BUILD_PLATFORM)
    EXECUTE_PROCESS(COMMAND uname -m
                    OUTPUT_VARIABLE BUILD_PLATFORM OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF(NOT DEFINED BUILD_PLATFORM)

# # set cxx flags
SET(CMAKE_CXX_FLAGS       "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -g -march=native")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -O0 -g -std=c++11 -march=native")

# # set directory
SET(SNOW_ROOT       ${PROJECT_SOURCE_DIR})
SET(SNOW_OUTPUT_DIR "/home/chaiyujin/Documents/GitHub/snow/libsnow")
SET(SNOW_OUTPUT_INC ${SNOW_OUTPUT_DIR}/include)
SET(SNOW_OUTPUT_LIB ${SNOW_OUTPUT_DIR}/${BUILD_PLATFORM}/${CMAKE_BUILD_TYPE}/lib)
SET(SNOW_OUTPUT_BIN ${SNOW_OUTPUT_DIR}/${BUILD_PLATFORM}/${CMAKE_BUILD_TYPE}/bin)
# ^=== prepare necessary macros ===^

# v=== get snow_core ===v
ADD_SUBDIRECTORY("snow/core")
# ^=== get snow_core ===^

# v=== install snow ===v
# # core module
ADD_LIBRARY(snow-core SHARED ${SNOW_CORE_HEADERS} ${SNOW_CORE_SOURCES})
TARGET_INCLUDE_DIRECTORIES(snow-core PUBLIC
    "$<BUILD_INTERFACE:${SNOW_ROOT}>"
    "$<INSTALL_INTERFACE:${SNOW_OUTPUT_INC}>")
# # entire snow
ADD_LIBRARY(snow SHARED ${SNOW_SOURCES})
SET_TARGET_PROPERTIES(snow PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(snow snow-core)
TARGET_INCLUDE_DIRECTORIES(snow PUBLIC
    "$<BUILD_INTERFACE:${SNOW_ROOT}>"
    "$<INSTALL_INTERFACE:${SNOW_OUTPUT_INC}>")
# # all headers
INSTALL(DIRECTORY ./snow/
        DESTINATION "${SNOW_OUTPUT_INC}/snow"
        FILES_MATCHING
            PATTERN "*.h"
            PATTERN "*.hpp"
            PATTERN "LICENSE"
)
# # install lib
INSTALL(TARGETS snow-core
    EXPORT core
    LIBRARY  DESTINATION "${SNOW_OUTPUT_LIB}"
    ARCHIVE  DESTINATION "${SNOW_OUTPUT_LIB}"
    RUNTIME  DESTINATION "${SNOW_OUTPUT_BIN}"
    INCLUDES DESTINATION "${SNOW_OUTPUT_INC}"
)
INSTALL(TARGETS snow snow-core
    EXPORT snow
    LIBRARY  DESTINATION "${SNOW_OUTPUT_LIB}"
    ARCHIVE  DESTINATION "${SNOW_OUTPUT_LIB}"
    RUNTIME  DESTINATION "${SNOW_OUTPUT_BIN}"
    INCLUDES DESTINATION "${SNOW_OUTPUT_INC}"
)
# # install config
INSTALL(EXPORT core
        FILE snow-core-config.cmake
        DESTINATION "${SNOW_OUTPUT_DIR}")
INSTALL(EXPORT snow
        FILE snow-config.cmake
        DESTINATION "${SNOW_OUTPUT_DIR}")
# ^=== install snow ===^

# v=== export for using add_subdirectory() ===v
export(TARGETS snow-core snow FILE snow-target.cmake)
# ^=== export for using add_subdirectory() ===^
