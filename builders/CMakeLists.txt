set(target builders)

add_library(
    ${target}
    STATIC
    cpp/vulkan_builder.cpp
)

target_include_directories(
    ${target}
    PUBLIC
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(
    ${target}
    PRIVATE
    vulkan
    glfw

    window
)