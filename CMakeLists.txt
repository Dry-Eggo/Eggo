cmake_minimum_required(VERSION 3.10)
project(EGGO)

set(CMAKE_BUILD_TYPE Debug)

add_executable(eggo src/eggo.cpp)
set(CMAKE_CXX_STANDDARD 17)

target_include_directories(eggo PUBLIC ${PROJECT_SOURCE_DIR}/headers /usr/include/c++/13 /usr/include/x86_64-linux-gnu/c++/13 /usr/include)
