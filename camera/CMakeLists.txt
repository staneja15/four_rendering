set(target camera)

add_library(
    ${target}
    STATIC
    cpp/camera.cpp
)

target_include_directories(
    ${target}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)