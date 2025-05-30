#include "graphics_pipeline.h"
#include "utils/error.h"

namespace fr {
    GraphicsPipeline::GraphicsPipeline(std::shared_ptr<VkContext>& context)
        : _context(context)
    { }

    void GraphicsPipeline::create_pipeline(const VertexInfo& vertex_info, std::vector<VkPipelineShaderStageCreateInfo>& shader_stages) {
        // Create a dynamic pipeline
        VkPipelineLayoutCreateInfo pipeline_layout_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &_context->descriptor.layout
        };
        validate(
            vkCreatePipelineLayout(_context->device, &pipeline_layout_info, nullptr, &_context->pipeline_layout),
            "Failed to create pipeline layout!"
        );

        // Specify that we will use triangle lists for drawing the geometry
        VkPipelineInputAssemblyStateCreateInfo input_assembly {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE  // note: this is useful when wanting to restart an index
        };

        // Specify the rasterisation state
        VkPipelineRasterizationStateCreateInfo raster {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable        = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode             = VK_POLYGON_MODE_FILL,
            .depthBiasEnable         = VK_FALSE,
            .lineWidth               = 1.0f
        };

        std::vector<VkDynamicState> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_CULL_MODE,
            VK_DYNAMIC_STATE_FRONT_FACE,
            VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
            VK_DYNAMIC_STATE_POLYGON_MODE_EXT
        };

        // Enable RGBA colour channels, but no blending is enabled
        VkPipelineColorBlendAttachmentState blend_attachment {
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo blend {
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments    = &blend_attachment
        };

        // Define 1 viewport and 1 scissor box
        VkPipelineViewportStateCreateInfo viewport {
            .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount  = 1
        };

        // Disable depth testing
        VkPipelineDepthStencilStateCreateInfo depth_stencil {
            .sType          = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthCompareOp = VK_COMPARE_OP_ALWAYS
        };

        // No multisampling.
        VkPipelineMultisampleStateCreateInfo multisample {
            .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
        };


        VkPipelineDynamicStateCreateInfo dynamic_state_info {
            .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
            .pDynamicStates    = dynamic_states.data()
        };

        // Pipeline rendering info (for dynamic rendering).
        VkPipelineRenderingCreateInfo pipeline_rendering_info {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount    = 1,
            .pColorAttachmentFormats = &_context->swap_chain_dimensions.format
        };

        // Create the graphics pipeline.
        VkGraphicsPipelineCreateInfo pipe {
            .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext               = &pipeline_rendering_info,
            .stageCount          = static_cast<std::uint32_t>(shader_stages.size()),
            .pStages             = shader_stages.data(),
            .pVertexInputState   = &vertex_info.vertex_info,
            .pInputAssemblyState = &input_assembly,
            .pViewportState      = &viewport,
            .pRasterizationState = &raster,
            .pMultisampleState   = &multisample,
            .pDepthStencilState  = &depth_stencil,
            .pColorBlendState    = &blend,
            .pDynamicState       = &dynamic_state_info,
            .layout              = _context->pipeline_layout,        // We need to specify the pipeline layout description up front as well.
            .renderPass          = VK_NULL_HANDLE,                   // Since we are using dynamic rendering this will set as null
            .subpass             = 0
        };

        validate(
            vkCreateGraphicsPipelines(_context->device, VK_NULL_HANDLE, 1, &pipe, nullptr, &_context->pipeline),
            "Failed to create graphics pipeline."
        );
    }
}
