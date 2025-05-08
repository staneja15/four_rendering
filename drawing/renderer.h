#pragma once
#include "builders/vulkan_structures.h"

namespace fr {
    class Renderer {
    public:
        Renderer(std::shared_ptr<VkContext>& context);

        bool record_command_buffer(std::size_t vertices_size);

        void present_image(std::uint32_t index);

    private:
        std::shared_ptr<VkContext> _context;

        VkResult _acquire_next_swap_chain_image(std::uint32_t* image);

        void _transition_image_layout(
            VkCommandBuffer       cmd,
            VkImage               image,
            VkImageLayout         oldLayout,
            VkImageLayout         newLayout,
            VkAccessFlags2        srcAccessMask,
            VkAccessFlags2        dstAccessMask,
            VkPipelineStageFlags2 srcStage,
            VkPipelineStageFlags2 dstStage
        );

        bool _resize();
    };
}
