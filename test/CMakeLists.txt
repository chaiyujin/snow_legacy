cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 11)

# message(STATUS ${CMAKE_INSTALL_PREFIX})
# LIST(APPEND CMAKE_PREFIX_PATH "../../build/")
# find_package(snow)

add_subdirectory(../project ext/libsnow/)

add_executable(main main.cpp)
target_link_libraries(main snow-core)
