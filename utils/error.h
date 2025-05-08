#pragma once

namespace fr {
    inline void validate(VkResult result, const std::string& error_message) {
        if (result != VK_SUCCESS) {
            throw std::runtime_error(error_message);
        }
    }
}  // namespace fr