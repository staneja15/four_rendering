#pragma once
#include "builders/vulkan_structures.h"

namespace fr {
    struct RendererParams {
        VkPolygonMode polygon_mode;
    };

    class Renderer {
    public:
        Renderer(std::shared_ptr<VkContext>& context);

        void build_command_buffers(const RendererParams& renderer_params);

        bool draw();

        void present_image(std::uint32_t index);

    private:
        std::shared_ptr<VkContext> _context;

        VkResult _acquire_next_swap_chain_image(std::uint32_t* image);

        void _transition_image_layout(
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

        bool _resize();
    };
}
