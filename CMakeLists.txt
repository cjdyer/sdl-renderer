cmake_minimum_required(VERSION 3.8)
project(sdl_renderer)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/yaml-cpp)

include_directories(include)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp SDL3::SDL3)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.yaml ${CMAKE_CURRENT_BINARY_DIR}/config.yaml COPYONLY)