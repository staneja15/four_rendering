set(target_name window)

add_library(
    ${target_name}
    STATIC
    cpp/GLFW_window.cpp
)

target_include_directories(
    ${target_name}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    ${target_name}
    PRIVATE
    camera
)
