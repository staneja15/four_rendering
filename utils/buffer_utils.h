#pragma once

#include "../builders/vulkan_structures.h"

namespace fr {
    class BufferUtils {
    public:
        BufferUtils(const std::shared_ptr<VkContext>& context);

        void create_buffer(BufferCore& buffer, VkDeviceSize buffer_size, VkBufferUsageFlags usage);

        void create_staging_buffer(BufferCore& buffer, VkDeviceSize buffer_size);

    private:
        std::shared_ptr<VkContext> _context;
    };
}