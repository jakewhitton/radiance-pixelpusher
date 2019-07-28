cmake_minimum_required(VERSION 2.8.9)
project(radiance-pixelpusher)

include_directories("lib/BlockingCollection" "src")

set(CMAKE_CXX_FLAGS "-Wall -pthread")

# Import SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Used to allow src/misc/Log.h to truncate source file indicator
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(run.exe ${SOURCES})
target_link_libraries(run.exe ${SDL2_LIBRARIES} SDL2_ttf)

set_property(TARGET run.exe PROPERTY CXX_STANDARD 17)
