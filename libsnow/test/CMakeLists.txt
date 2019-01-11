cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 11)

include_directories(../bin/x86_64/Debug/include/)
link_directories(../bin/x86_64/Debug/lib)

add_executable(main main.cpp)
target_link_libraries(main snow_core)
