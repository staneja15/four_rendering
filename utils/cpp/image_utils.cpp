#include "image_utils.h"

namespace fr::image {
    void transition_layout(
        VkCommandBuffer cmd,
        VkImage image,
        VkImageLayout old_layout,
        VkImageLayout new_layout,
        VkImageAspectFlags image_flag_bits,
        VkAccessFlags2 src_access_mask,
        VkAccessFlags2 dst_access_mask,
        VkPipelineStageFlags2 src_stage,
        VkPipelineStageFlags2 dst_stage
    ) {
        VkImageMemoryBarrier2 image_barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,

            // Specify the pipeline stages and access masks for the barrier
            .srcStageMask  = src_stage,             // Source pipeline stage mask
            .srcAccessMask = src_access_mask,        // Source access mask
            .dstStageMask  = dst_stage,             // Destination pipeline stage mask
            .dstAccessMask = dst_access_mask,        // Destination access mask

            // Specify the old and new layouts of the image
            .oldLayout = old_layout,        // Current layout of the image
            .newLayout = new_layout,        // Target layout of the image

            // We are not changing the ownership between queues
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

            // Specify the image to be affected by this barrier
            .image = image,

            // Define the subresource range (which parts of the image are affected)
            .subresourceRange = {
                .aspectMask     = image_flag_bits,        // Affects the color aspect of the image
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
}  // namespace fr
