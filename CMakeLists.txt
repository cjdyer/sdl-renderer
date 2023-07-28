cmake_minimum_required(VERSION 3.8)
project(sdl_renderer)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL)

include_directories(include)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/camera.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)