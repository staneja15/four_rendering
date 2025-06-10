#pragma once

#include "builders/vulkan_structures.h"

namespace fr {
    class ScopedCommandBuffer {
    public:
        ScopedCommandBuffer(const std::shared_ptr<VkContext>& context);

        ~ScopedCommandBuffer();

        void begin();

        [[nodiscard]] VkCommandBuffer get_command_buffer() const;

    private:
        VkDevice _device;
        std::shared_ptr<VkContext> _context;
        VkCommandPool _command_pool {};
        VkCommandBuffer _command_buffer {};
    };
}  // namespace fr