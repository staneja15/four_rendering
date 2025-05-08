# Project settings
cmake_minimum_required(VERSION 3.25)
project(
    four_rendering
    VERSION 0.0.1
    LANGUAGES CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Setup the Project interface
add_library(
    ${PROJECT_NAME}
    INTERFACE
)

target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE

    builders
    window
    camera
    drawing
    shaders
)

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
    ${PROJECT_SOURCE_DIR}
)

add_subdirectory(builders)
add_subdirectory(utils)
add_subdirectory(window)
add_subdirectory(camera)
add_subdirectory(drawing)
add_subdirectory(shaders)

# Setup the example application
add_executable(
    ${PROJECT_NAME}_run
    main.cpp
    sample_application.cpp
)

target_link_libraries(
    ${PROJECT_NAME}_run
    PUBLIC
    four::rendering
)

target_include_directories(
    ${PROJECT_NAME}_run
    PUBLIC
    ${PROJECT_SOURCE_DIR}
)

add_library(four::rendering ALIAS ${PROJECT_NAME})

# --------
# Testing
find_package(Catch2 3 REQUIRED)


add_executable(
    ${PROJECT_NAME}_test
)

target_sources(
    ${PROJECT_NAME}_test
    PRIVATE
    main.cpp  # todo: remove when first test is added as source
)

target_link_libraries(
    ${PROJECT_NAME}_test
    PRIVATE
    Catch2::Catch2WithMain
)


