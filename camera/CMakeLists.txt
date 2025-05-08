set(target camera)

add_library(
    ${target}
    STATIC
    cpp/camera.cpp
)

target_include_directories(
    ${target}
    PRIVATE
    ${PROJECT_SOURCE_DIR}
)