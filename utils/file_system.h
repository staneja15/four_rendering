#pragma once

#include <vector>
#include <filesystem>

namespace fr::file_system {
    std::vector<char> read_binary_file(const std::filesystem::path& filename);
}  // namespace four::file_system