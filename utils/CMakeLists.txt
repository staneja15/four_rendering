set(target utils)

add_library(
    ${target}
    STATIC
    cpp/file_system.cpp
)

target_include_directories(
    ${target}
    PRIVATE
    ${PROJECT_SOURCE_DIR}
)