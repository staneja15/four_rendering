#pragma once

#include "builders/vulkan_structures.h"
#include "vertex_types.h"

#include <memory>

#include <vulkan/vulkan.h>

namespace fr {
    class GraphicsPipeline {
    public:
        explicit GraphicsPipeline(std::shared_ptr<VkContext>& context);

        void create_pipeline(const VertexInfo& vertex_info, std::vector<VkPipelineShaderStageCreateInfo>& shader_stages);

    private:
        std::shared_ptr<VkContext> _context;
    };
}
