#include "utils/file_system.h"

#include <fstream>

namespace fr::file_system {
    std::vector<char> read_binary_file(const std::filesystem::path& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file!");
        }

        const std::streamsize file_size = file.tellg();
        std::vector<char> buffer(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);
        file.close();

        return buffer;
    }
}  // namespace four::file_system