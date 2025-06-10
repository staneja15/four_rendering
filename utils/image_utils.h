#pragma once

#include <vulkan/vulkan.h>

namespace fr::image {
    void transition_layout(
        VkCommandBuffer       cmd,
        VkImage               image,
        VkImageLayout         old_layout,
        VkImageLayout         new_layout,
        VkImageAspectFlags image_flag_bits,
        VkAccessFlags2        src_access_mask,
        VkAccessFlags2        dst_access_mask,
        VkPipelineStageFlags2 src_stage,
        VkPipelineStageFlags2 dst_stage
    );
}  // namespace fr