cmake_minimum_required(VERSION 2.8.9)
project(radiance-pixelpusher)

include_directories("lib/BlockingCollection")

set(CMAKE_CXX_FLAGS "-Wall -pthread")

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

file(GLOB SOURCES "src/*.cpp")

add_executable(run.exe ${SOURCES})
