cmake_minimum_required(VERSION 3.0.0)
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
SET(SNOW_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin/${BUILD_PLATFORM}/${CMAKE_BUILD_TYPE})
SET(SNOW_OUTPUT_INC ${SNOW_OUTPUT_DIR}/include/snow)
SET(SNOW_OUTPUT_LIB ${SNOW_OUTPUT_DIR}/lib)
# ^=== prepare necessary macros ===^

# v=== build core module ===v
ADD_SUBDIRECTORY("./core")
# ^=== build core module ===^

INSTALL(FILES snow.h DESTINATION "${SNOW_OUTPUT_INC}")
