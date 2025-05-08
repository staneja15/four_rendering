set(target drawing)

add_library(
    ${target}
    STATIC
    cpp/graphics_pipeline.cpp
    cpp/renderer.cpp
    cpp/vertex_buffer.cpp
)

target_include_directories(
    ${target}
    PRIVATE
    ${PROJECT_SOURCE_DIR}
)
