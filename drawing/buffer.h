#pragma once

#include "builders/vulkan_structures.h"

namespace fr {
    class Buffer {
    public:
        Buffer(std::shared_ptr<VkContext>& context);

        void create_buffer(BufferCore& buffer, void* vertices, VkDeviceSize buffer_size, VkBufferUsageFlags usage, bool unmap);

    private:
        std::shared_ptr<VkContext> _context;

        std::uint32_t _find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);
    };
}