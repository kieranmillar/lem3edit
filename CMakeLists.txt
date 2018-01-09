cmake_minimum_required(VERSION 2.6)
project(lem3edit)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

file(GLOB_RECURSE lem3editSources "src/*.cpp")
include_directories(${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR})

add_executable(lem3edit ${lem3editSources})
target_link_libraries(lem3edit ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES})
