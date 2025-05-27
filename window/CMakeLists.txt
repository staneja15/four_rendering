set(target_name window)

add_library(
    ${target_name}
    STATIC
    cpp/GLFW_window.cpp
)

target_include_directories(
    ${target_name}
    PRIVATE
    ${FR_ROOT_DIR}
)

target_link_libraries(
    ${target_name}
    PRIVATE
    camera
)
