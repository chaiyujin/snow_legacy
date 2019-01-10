cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 11)

include_directories(../libsnow/)
add_executable(test_mem  test_mem.cpp ../libsnow/core/utils/log.cpp)