cmake_minimum_required(VERSION 3.8)
project(sdl_renderer)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)
