set(target shaders)

add_library(
    ${target}
    STATIC
    cpp/shader.cpp
)

target_include_directories(
    ${target}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    ${target}
    PRIVATE
    fr_utils
)