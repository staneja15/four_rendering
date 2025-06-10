set(target builders)

add_library(
    ${target}
    STATIC
    cpp/vulkan_builder.cpp
    cpp/texture_loader.cpp
)

target_include_directories(
    ${target}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    ${target}
    PRIVATE
    vulkan
    glfw
    fr_utils

    window
)