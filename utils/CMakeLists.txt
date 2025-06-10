set(target fr_utils)

add_library(
    ${target}
    STATIC
    cpp/file_system.cpp
    cpp/scoped_command_buffer.cpp
    cpp/buffer_utils.cpp
    cpp/image_utils.cpp
)

target_include_directories(
    ${target}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)