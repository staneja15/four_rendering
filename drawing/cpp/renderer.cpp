#include "drawing/renderer.h"
#include "utils/error.h"

namespace fr {
    Renderer::Renderer(std::shared_ptr<VkContext>& context)
        : _context(context)
    { }

    bool Renderer::record_command_buffer(std::size_t vertices_size) {
        std::uint32_t index = 0;
        auto res = _acquire_next_swap_chain_image(&index);

        // handle outdated error in acquire swap chain image
        if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
            if (!_resize()) {
                throw std::runtime_error("Failed to resize window.");
            }
            return false;
        } else if (res != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image.");
        }

        VkCommandBuffer cmd = _context->per_frame[index].primary_command_buffer;

        VkCommandBufferBeginInfo begin_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        // Begin command buffer recording
        validate(
            vkBeginCommandBuffer(cmd, &begin_info),
            "Failed to start recording command buffer."
        );

        // transition the image to the COLOR_ATTACHMENT_OPTIMAL for drawing
        _transition_image_layout(
            cmd,
            _context->swap_chain_images[index],
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            0,                                                     // srcAccessMask (no need to wait for previous operations)
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,                // dstAccessMask
            VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,                   // srcStage
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT        // dstStage
        );

        // Set clear color values.
        VkClearValue clear_value {
            .color = {{0.01f, 0.01f, 0.033f, 1.0f}}
        };

        // Set up the rendering attachment info
        VkRenderingAttachmentInfo color_attachment {
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView   = _context->swap_chain_image_views[index],
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue  = clear_value
        };

        // Begin rendering
        VkRenderingInfo rendering_info {
            .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea           = {    // Initialize the nested `VkRect2D` structure
                .offset = {0, 0},        // Initialize the `VkOffset2D` inside `renderArea`
                .extent = {              // Initialize the `VkExtent2D` inside `renderArea`
                    .width  = _context->swap_chain_dimensions.width,
                    .height = _context->swap_chain_dimensions.height}
                },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments    = &color_attachment
        };

        vkCmdBeginRendering(cmd, &rendering_info);

        // bind the graphics pipeline
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _context->pipeline);

        // Set the dynamic states (defined in the pipeline creation)

        VkViewport vp {
            .width    = static_cast<float>(_context->swap_chain_dimensions.width),
            .height   = static_cast<float>(_context->swap_chain_dimensions.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        vkCmdSetViewport(cmd, 0, 1, &vp);

        VkRect2D scissor {
            .extent = {
                .width  = _context->swap_chain_dimensions.width,
                .height = _context->swap_chain_dimensions.height
            }
        };
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
        vkCmdSetFrontFace(cmd, VK_FRONT_FACE_CLOCKWISE);
        vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        _context->extensions.polygon_mode(cmd, VK_POLYGON_MODE_FILL);

        VkDeviceSize offset = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, &_context->vertex_buffer, &offset);
        vkCmdDraw(cmd, vertices_size, 1, 0, 0);

        // Complete rendering
        vkCmdEndRendering(cmd);

        // After rendering, transition to the PRESENT_SRC layout
        _transition_image_layout(
            cmd,
            _context->swap_chain_images[index],
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,                 // srcAccessMask
            0,                                                      // dstAccessMask
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,        // srcStage
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
        );

        validate(
            vkEndCommandBuffer(cmd),
            "Failed to complete the command buffer."
        );

        // Submit it to the queue with a release semaphore.
        if (_context->per_frame[index].swap_chain_release_semaphore == VK_NULL_HANDLE) {
            VkSemaphoreCreateInfo semaphore_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            validate(
                vkCreateSemaphore(_context->device, &semaphore_info, nullptr, &_context->per_frame[index].swap_chain_release_semaphore),
                "Failed to create release semaphore."
            );
        }

        // Use top of pipe bit to ensure that no parts of the pipeline execute until the swap chain image is
        //      acquired (signalled by the acquire_semaphore).
        VkPipelineStageFlags wait_stage { VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT };
        VkSubmitInfo info {
            .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount   = 1,
            .pWaitSemaphores      = &_context->per_frame[index].swap_chain_acquire_semaphore,
            .pWaitDstStageMask    = &wait_stage,
            .commandBufferCount   = 1,
            .pCommandBuffers      = &cmd,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &_context->per_frame[index].swap_chain_release_semaphore
        };

        validate(
            vkQueueSubmit(_context->queue, 1, &info, _context->per_frame[index].queue_submit_fence),
            "Failed to submid command buffer to graphics queue."
        );

        present_image(index);

        return true;
    }

    void Renderer::present_image(std::uint32_t index) {
        VkPresentInfoKHR present{
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &_context->per_frame[index].swap_chain_release_semaphore,
            .swapchainCount     = 1,
            .pSwapchains        = &_context->swap_chain,
            .pImageIndices      = &index,
        };

        // Present swapchain image
        vkQueuePresentKHR(_context->queue, &present);
    }

    VkResult Renderer::_acquire_next_swap_chain_image(std::uint32_t* image) {
        VkSemaphore acquire_semaphore;
        if (_context->recycled_semaphores.empty()) {
            VkSemaphoreCreateInfo info {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            validate(
                vkCreateSemaphore(_context->device, &info, nullptr, &acquire_semaphore),
                "Failed to create semaphore."
            );
        } else {
            acquire_semaphore = _context->recycled_semaphores.back();
            _context->recycled_semaphores.pop_back();
        }

        VkResult res = vkAcquireNextImageKHR(_context->device, _context->swap_chain, UINT64_MAX, acquire_semaphore, VK_NULL_HANDLE, image);

        if (res != VK_SUCCESS) {
            // Failed to acquire the next image
            _context->recycled_semaphores.push_back(acquire_semaphore);
            return res;
        }

        // Wait for submit fences to finish before reusing rendering resources
        if (_context->per_frame[*image].queue_submit_fence != VK_NULL_HANDLE) {
            vkWaitForFences(_context->device, 1, &_context->per_frame[*image].queue_submit_fence, true, UINT64_MAX);
            vkResetFences(_context->device, 1, &_context->per_frame[*image].queue_submit_fence);
        }

        if (_context->per_frame[*image].primary_command_pool != VK_NULL_HANDLE) {
            vkResetCommandPool(_context->device, _context->per_frame[*image].primary_command_pool, 0);
        }

        VkSemaphore old_semaphore = _context->per_frame[*image].swap_chain_acquire_semaphore;
        if (old_semaphore != VK_NULL_HANDLE) {
            _context->recycled_semaphores.push_back(old_semaphore);
        }

        _context->per_frame[*image].swap_chain_acquire_semaphore = acquire_semaphore;

        return VK_SUCCESS;
    }

    void Renderer::_transition_image_layout(
        VkCommandBuffer cmd,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkAccessFlags2 srcAccessMask,
        VkAccessFlags2 dstAccessMask,
        VkPipelineStageFlags2 srcStage,
        VkPipelineStageFlags2 dstStage
    ) {
        VkImageMemoryBarrier2 image_barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,

            // Specify the pipeline stages and access masks for the barrier
            .srcStageMask  = srcStage,             // Source pipeline stage mask
            .srcAccessMask = srcAccessMask,        // Source access mask
            .dstStageMask  = dstStage,             // Destination pipeline stage mask
            .dstAccessMask = dstAccessMask,        // Destination access mask

            // Specify the old and new layouts of the image
            .oldLayout = oldLayout,        // Current layout of the image
            .newLayout = newLayout,        // Target layout of the image

            // We are not changing the ownership between queues
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

            // Specify the image to be affected by this barrier
            .image = image,

            // Define the subresource range (which parts of the image are affected)
            .subresourceRange = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,        // Affects the color aspect of the image
                .baseMipLevel   = 0,                                // Start at mip level 0
                .levelCount     = 1,                                // Number of mip levels affected
                .baseArrayLayer = 0,                                // Start at array layer 0
                .layerCount     = 1                                 // Number of array layers affected
            }
        };

        // Initialize the VkDependencyInfo structure
        VkDependencyInfo dependency_info {
            .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags         = 0,                    // No special dependency flags
            .imageMemoryBarrierCount = 1,                    // Number of image memory barriers
            .pImageMemoryBarriers    = &image_barrier        // Pointer to the image memory barrier(s)
        };

        // Record the pipeline barrier into the command buffer
        vkCmdPipelineBarrier2(cmd, &dependency_info);
    }

    bool Renderer::_resize() {
        if (_context->device == VK_NULL_HANDLE) {
            return false;
        }

        VkSurfaceCapabilitiesKHR surface_properties;
        validate(
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_context->gpu, _context->surface, &surface_properties),
            "Failed to obtain surface properties."
        );

        // Only re-create swap chain if the dimensions have changed
        if (surface_properties.currentExtent.width == _context->swap_chain_dimensions.width &&
            surface_properties.currentExtent.height == _context->swap_chain_dimensions.height) {
            return false;
        }

        vkDeviceWaitIdle(_context->device);


        return true;
    }
}
