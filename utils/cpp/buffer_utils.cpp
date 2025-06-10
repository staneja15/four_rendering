#include "buffer_utils.h"
#include "error.h"

#include <ranges>

namespace fr {
    BufferUtils::BufferUtils(const std::shared_ptr<VkContext>& context)
        : _context(context)
    { }

    void BufferUtils::create_buffer(BufferCore& buffer, VkDeviceSize buffer_size, const VkBufferUsageFlags usage) {
        if (buffer.buffer == VK_NULL_HANDLE) {
            // Create the buffer
            buffer.size = buffer_size;

            VkBufferCreateInfo buffer_info {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size  = buffer_size,
                .usage = usage
            };

            VmaAllocationCreateInfo alloc_info {
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO
            };

            validate(
                vmaCreateBuffer(_context->allocator, &buffer_info, &alloc_info, &buffer.buffer, &buffer.allocation, nullptr),
                "Failed to create VMA buffer"
            );
        }
    }

    void BufferUtils::create_staging_buffer(BufferCore& buffer, VkDeviceSize buffer_size) {
        if (buffer.buffer == VK_NULL_HANDLE) {
            // Create the buffer
            buffer.size = buffer_size;

            VkBufferCreateInfo buffer_info {
                .sType =       VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size  =       buffer_size,
                .usage =       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE
            };

            VmaAllocationCreateInfo alloc_info {
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO
            };

            validate(
                vmaCreateBuffer(_context->allocator, &buffer_info, &alloc_info, &buffer.buffer, &buffer.allocation, nullptr),
                "Failed to create VMA staging buffer"
            );
        }
    }
}
