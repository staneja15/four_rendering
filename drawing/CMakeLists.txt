set(target drawing)

add_library(
    ${target}
    STATIC
    cpp/graphics_pipeline.cpp
    cpp/renderer.cpp
    cpp/buffer.cpp
    cpp/descriptor_set.cpp
)

target_include_directories(
    ${target}
    PRIVATE
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(
    ${target}
    PRIVATE
    camera
)
