cmake_minimum_required(VERSION 3.13)
project("vde_switch"
    DESCRIPTION "this is it no mo iterations"
    VERSION 0.0.1
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Debug)

include_directories(include src)

file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(vde_switch ${SOURCES})

#add_subdirectory(tools)
#add_subdirectory(external/vdeplug4)


target_include_directories(vde_switch PUBLIC
                           "${PROJECT_BINARY_DIR}")

target_link_libraries(vde_switch)