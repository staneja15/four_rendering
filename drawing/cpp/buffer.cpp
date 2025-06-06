#include "buffer.h"
#include "utils/error.h"

#include <ranges>

namespace fr {
    Buffer::Buffer(const std::shared_ptr<VkContext>& context)
        : _context(context)
    { }

    void Buffer::create_buffer(BufferCore& buffer, void* data, VkDeviceSize buffer_size, const VkBufferUsageFlags usage) {
        if (buffer.buffer == VK_NULL_HANDLE) {
            // Create the buffer
            buffer.size = buffer_size;

            VkBufferCreateInfo buffer_info {
                .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size        = buffer_size,
                .usage       = usage
            };

            VmaAllocationCreateInfo alloc_info {
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO
            };

            validate(
                vmaCreateBuffer(_context->allocator, &buffer_info, &alloc_info, &buffer.buffer, &buffer.allocation, nullptr),
                "Failed to create VMA Buffer"
            );
        }
    }

    std::size_t Buffer::calculate_dynamic_alignment(const std::size_t size) const {
        std::size_t dynamic_alignment = size;

        // Get physical device properties
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(_context->gpu, &properties);

        // Set the dynamic alignment
        std::uint32_t min_ubo_alignment = properties.limits.minUniformBufferOffsetAlignment;
        if (min_ubo_alignment > 0) {
            dynamic_alignment = (dynamic_alignment + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);
        }

        return dynamic_alignment;
    }

    std::uint32_t Buffer::_find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties) {
        // Structure to hold the physical device's memory properties
        VkPhysicalDeviceMemoryProperties mem_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

        // Iterate over all memory types available on the physical device
        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
        {
            // Check if the current memory type is acceptable based on the type_filter
            // The type_filter is a bitmask where each bit represents a memory type that is suitable
            if (type_filter & (1 << i))
            {
                // Check if the memory type has all the desired property flags
                // properties is a bitmask of the required memory properties
                if ((mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    // Found a suitable memory type; return its index
                    return i;
                }
            }
        }

        // If no suitable memory type was found, throw an exception
        throw std::runtime_error("Failed to find suitable memory type.");
    }
}
