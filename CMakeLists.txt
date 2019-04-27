cmake_minimum_required(VERSION 2.8.9)
project(radiance-pixelpusher)

set(CMAKE_CXX_FLAGS "-Wall -pthread")

file(GLOB SOURCES "src/*.cpp")

add_executable(run.exe ${SOURCES})
