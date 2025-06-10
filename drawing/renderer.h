#pragma once
#include "builders/vulkan_structures.h"

namespace fr {
    struct RendererParams {
        bool instance = false;  // If using instancing, set this to true.
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

        bool _resize();
    };
}
