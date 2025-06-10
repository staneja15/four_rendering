#include "renderer.h"
#include "descriptor_set_types.h"
#include "utils/error.h"
#include "utils/image_utils.h"

#include <glm/glm.hpp>

namespace fr {
    Renderer::Renderer(std::shared_ptr<VkContext>& context)
        : _context(context)
    { }

    void Renderer::build_command_buffers(const RendererParams& renderer_params) {
        VkCommandBufferBeginInfo begin_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };

        for (std::size_t frame = 0; frame < _context->per_frame.size(); ++frame) {
            vkWaitForFences(_context->device, 1, &_context->per_frame[frame].queue_submit_fence, VK_TRUE, UINT64_MAX);

            // Begin command buffer recording
            validate(
                vkBeginCommandBuffer(_context->per_frame[frame].primary_command_buffer, &begin_info),
                "Failed to start recording command buffer."
            );

            // transition the image to the COLOR_ATTACHMENT_OPTIMAL for drawing
            image::transition_layout(
                _context->per_frame[frame].primary_command_buffer,
                _context->swap_chain_images[frame],
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,                                                     // srcAccessMask (no need to wait for previous operations)
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,                // dstAccessMask
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,                   // srcStage
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT        // dstStage
            );

            image::transition_layout(
                _context->per_frame[frame].primary_command_buffer,
                _context->depth_image,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                0,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
            );

            // Set clear color values.
            const VkClearValue clear_value {
                .color = {{0.01f, 0.01f, 0.033f, 1.0f}}
            };

            // Set up the rendering attachment info
            VkRenderingAttachmentInfo color_attachment {
                .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView   = _context->swap_chain_image_views[frame],
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue  = clear_value
            };

            VkRenderingAttachmentInfo depth_attachment {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = _context->depth_image_view,
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue  = {1.0f, 0}
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
            .pColorAttachments    = &color_attachment,
            .pDepthAttachment     = &depth_attachment
        };

            vkCmdBeginRendering(_context->per_frame[frame].primary_command_buffer, &rendering_info);

            // bind the graphics pipeline
            vkCmdBindPipeline(_context->per_frame[frame].primary_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _context->pipeline);

            // Set the dynamic states (defined in the pipeline creation)
            VkViewport vp {
                .width    = static_cast<float>(_context->swap_chain_dimensions.width),
                .height   = static_cast<float>(_context->swap_chain_dimensions.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f
            };
            vkCmdSetViewport(_context->per_frame[frame].primary_command_buffer, 0, 1, &vp);

            VkRect2D scissor {
                .extent = {
                    .width  = _context->swap_chain_dimensions.width,
                    .height = _context->swap_chain_dimensions.height
                }
            };
            vkCmdSetScissor(_context->per_frame[frame].primary_command_buffer, 0, 1, &scissor);

            vkCmdSetCullMode(_context->per_frame[frame].primary_command_buffer, VK_CULL_MODE_NONE);
            vkCmdSetFrontFace(_context->per_frame[frame].primary_command_buffer, VK_FRONT_FACE_CLOCKWISE);
            vkCmdSetPrimitiveTopology(_context->per_frame[frame].primary_command_buffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

            _context->extensions.polygon_mode(_context->per_frame[frame].primary_command_buffer, renderer_params.polygon_mode);

            VkDeviceSize offset = {0};
            vkCmdBindVertexBuffers(_context->per_frame[frame].primary_command_buffer, 0, 1, &_context->vertex_buffer.buffer, &offset);
            vkCmdBindIndexBuffer(_context->per_frame[frame].primary_command_buffer, _context->indices_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

            if (renderer_params.instance) {
                vkCmdBindVertexBuffers(_context->per_frame[frame].primary_command_buffer, 1, 1, &_context->instance_buffer.buffer, &offset);
            }

            vkCmdBindDescriptorSets(_context->per_frame[frame].primary_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _context->pipeline_layout, 0, 1, &_context->descriptor.descriptor, 0, nullptr);

            vkCmdDrawIndexed(_context->per_frame[frame].primary_command_buffer, _context->indices_buffer.count, _context->instance_count, 0, 0, 0);

            // Complete rendering
            vkCmdEndRendering(_context->per_frame[frame].primary_command_buffer);

            // After rendering, transition to the PRESENT_SRC layout
            image::transition_layout(
                _context->per_frame[frame].primary_command_buffer,
                _context->swap_chain_images[frame],
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,                   // srcAccessMask
                0,                                                        // dstAccessMask
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,          // srcStage
                VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                    // dstStage
            );

            validate(
                vkEndCommandBuffer(_context->per_frame[frame].primary_command_buffer),
                "Failed to complete the command buffer."
            );
        }
    }

    bool Renderer::draw() {
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
            .pCommandBuffers      = &_context->per_frame[index].primary_command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &_context->per_frame[index].swap_chain_release_semaphore
        };

        validate(
            vkQueueSubmit(_context->queue, 1, &info, _context->per_frame[index].queue_submit_fence),
            "Failed to submit command buffer to graphics queue."
        );

        present_image(index);

        return true;
    }

    void Renderer::present_image(std::uint32_t index) {
        VkPresentInfoKHR present {
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

        VkSemaphore old_semaphore = _context->per_frame[*image].swap_chain_acquire_semaphore;
        if (old_semaphore != VK_NULL_HANDLE) {
            _context->recycled_semaphores.push_back(old_semaphore);
        }

        _context->per_frame[*image].swap_chain_acquire_semaphore = acquire_semaphore;

        return VK_SUCCESS;
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
