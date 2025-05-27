set(target builders)

add_library(
    ${target}
    STATIC
    cpp/vulkan_builder.cpp
)

target_include_directories(
    ${target}
    PUBLIC
    ${FR_ROOT_DIR}
)

target_link_libraries(
    ${target}
    PRIVATE
    vulkan
    glfw

    window
)