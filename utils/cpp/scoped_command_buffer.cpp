#include "scoped_command_buffer.h"
#include "error.h"

/*
 *  Initialises a command buffer that will clean itself up when it falls out of scope.
 *
 *  Call begin() to begin recording the command buffer, and when it falls out of scope, it will automatically
 *      end and be submitted to the graphics queue (see destructor function for implementation details).
 */

namespace fr {
    ScopedCommandBuffer::ScopedCommandBuffer(const std::shared_ptr<VkContext>& context)
        : _device(context->device)
        , _context(context)
    {
        VkCommandPoolCreateInfo command_pool_info = {};
        command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        command_pool_info.queueFamilyIndex = _context->graphics_queue_index;

        validate(
            vkCreateCommandPool(_device, &command_pool_info, nullptr, &_command_pool),
            "Failed to create command pool!"
        );

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = _command_pool;
        alloc_info.commandBufferCount = 1;

        validate(
            vkAllocateCommandBuffers(_device, &alloc_info, &_command_buffer),
            "Failed to allocate scoped command buffer!"
        );
    }

    ScopedCommandBuffer::~ScopedCommandBuffer() {
        vkEndCommandBuffer(_command_buffer);

        // Submit the buffer to the queue
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &_command_buffer;

        vkQueueSubmit(_context->queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(_context->queue);  // Let the command buffer finish processing

        vkFreeCommandBuffers(_device, _command_pool, 1, &_command_buffer);
        vkDestroyCommandPool(_device, _command_pool, nullptr);
    }

    void ScopedCommandBuffer::begin() {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(_command_buffer, &begin_info);
    }

    VkCommandBuffer ScopedCommandBuffer::get_command_buffer() const {
        return _command_buffer;
    }
}  // namespace fr