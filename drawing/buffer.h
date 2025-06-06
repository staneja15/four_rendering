#pragma once

#include "builders/vulkan_structures.h"

namespace fr {
    class Buffer {
    public:
        Buffer(const std::shared_ptr<VkContext>& context);

        void create_buffer(BufferCore& buffer, void* data, VkDeviceSize buffer_size, VkBufferUsageFlags usage);

        std::size_t calculate_dynamic_alignment(std::size_t size) const;

    private:
        std::shared_ptr<VkContext> _context;

        std::uint32_t _find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);
    };
}