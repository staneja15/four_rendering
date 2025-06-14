# Find required packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Set global includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern)

# Setup the Project interface
add_library(
    four_rendering
    INTERFACE
)

target_link_libraries(
    four_rendering
    INTERFACE

    builders
    window
    camera
    drawing
    shaders
)

target_include_directories(
    four_rendering
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(builders)
add_subdirectory(utils)
add_subdirectory(window)
add_subdirectory(camera)
add_subdirectory(drawing)
add_subdirectory(shaders)

# Setup the example application
add_executable(
    four_rendering_run
    main.cpp
    sample_application.cpp
)

target_link_libraries(
    four_rendering_run
    PUBLIC
    four::rendering
)

target_include_directories(
    four_rendering_run
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(four::rendering ALIAS four_rendering)

