#include "drawing/vertex_buffer.h"

#include <ranges>

#include "utils/error.h"

namespace fr {
    VertexBuffer::VertexBuffer(std::shared_ptr<VkContext>& context)
        : _context(context)
    { }

    void VertexBuffer::create_vertex_buffer(void* vertices, VkDeviceSize buffer_size) {
        // Create the buffer
        VkBufferCreateInfo vertex_buffer_info {
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .flags       = 0,
            .size        = buffer_size,
            .usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };
        validate(
            vkCreateBuffer(_context->device, &vertex_buffer_info, nullptr, &_context->vertex_buffer),
            "Failed to create vertex buffer."
        );

        // Get memory requirements
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(_context->device, _context->vertex_buffer, &memory_requirements);

        // Allocate memory to the buffer
        // todo: We should map this to device local memory
        VkMemoryAllocateInfo alloc_info {
            .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize  = memory_requirements.size,
            .memoryTypeIndex = _find_memory_type(
                _context->gpu,
                memory_requirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            )
        };

        validate(
            vkAllocateMemory(_context->device, &alloc_info, nullptr, &_context->vertex_buffer_memory),
            "Failed to allocate vertex buffer memory."
        );

        // Bind the buffer with the allocated memory
        validate(
            vkBindBufferMemory(_context->device, _context->vertex_buffer, _context->vertex_buffer_memory, 0),
            "Failed to bind vertex buffer memory."
        );

        // Map the memory and copy the vertex data
        void* data;
        validate(
            vkMapMemory(_context->device, _context->vertex_buffer_memory, 0, buffer_size, 0, &data),
            "Failed to Map the vertex buffer memory."
        );
        memcpy(data, vertices, static_cast<std::size_t>(buffer_size));
        vkUnmapMemory(_context->device, _context->vertex_buffer_memory);
    }

    std::uint32_t VertexBuffer::_find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties) {
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
