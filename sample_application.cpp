#include "sample_application.h"

#include "drawing/graphics_pipeline.h"
#include "utils/buffer_utils.h"
#include "drawing/renderer.h"
#include "drawing/descriptor_set.h"
#include "drawing/descriptor_set_types.h"
#include "shaders/shader.h"

SampleApplication::SampleApplication()
    : _vulkan_builder(std::make_unique<fr::VulkanBuilder>())
    , _texture(std::make_unique<fr::Texture>(_vulkan_builder->get_context()))
{ }

void SampleApplication::init() {
    _vulkan_builder->prepare();
    _context = _vulkan_builder->get_context();

    // Create the vertex info
    fr::VertexInfo vertex_info {};
    constexpr fr::HelloTriangleVertex hello_triangle_vertex {};
    hello_triangle_vertex.generate_vertex_info(vertex_info, 0);

    // Create shader stages
    auto shader = fr::Shader("basic", _context->device);
    shader.create_shader_program();
    auto shader_stages = shader.get_shader_stages();

    // Set vertices data and descriptor sets
    set_uniforms();

    // Create the graphics pipeline
    auto terrain_pipeline = fr::GraphicsPipeline(_context);
    terrain_pipeline.create_pipeline(vertex_info, shader_stages);

    shader.destroy_shaders();  // shaders are now baked into the pipeline, we can now freely destroy them
}

void SampleApplication::set_uniforms() {
    auto indices = std::vector<std::uint32_t> {
        0, 1, 2,
        0, 2, 3
    };
    _context->indices_buffer.count = static_cast<std::uint32_t>(indices.size());

    auto buffer = fr::BufferUtils(_context);
    buffer.create_buffer(_context->vertex_buffer,  sizeof(fr::HelloTriangleVertex) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    buffer.create_buffer(_context->indices_buffer, sizeof(std::uint32_t)  * indices.size(),  VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    vmaCopyMemoryToAllocation(_context->allocator, vertices.data(), _context->vertex_buffer.allocation, 0, sizeof(fr::HelloTriangleVertex) * vertices.size());
    vmaCopyMemoryToAllocation(_context->allocator, indices.data(), _context->indices_buffer.allocation, 0, sizeof(std::uint32_t)  * indices.size());

    // Create descriptor sets
    buffer.create_buffer(_context->descriptor.uniform_buffer, sizeof(fr::ViewProj), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    _texture->load("/opt/four_map_engine/four_rendering/assets/images/yak.jpg");
    fr::TextureInfo texture_info = _texture->get_info();


    std::vector<fr::DescriptorInfo> infos = {
        fr::DescriptorInfo(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            _context->descriptor.uniform_buffer.size,
            _context->descriptor.uniform_buffer.buffer,
            0
        ),
        fr::DescriptorInfo(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            1,
            texture_info.size,
            texture_info.image_info,
            0
        )
    };

    auto descriptor_set = fr::DescriptorSet(_context);
    descriptor_set.create_descriptor_sets(_context->descriptor, infos);
}

void SampleApplication::run() {
    // Initialise the renderer
    auto renderer = fr::Renderer(_context);
    auto renderer_params = fr::RendererParams {
        .instance = false,
        .polygon_mode = VK_POLYGON_MODE_FILL
    };

    renderer.build_command_buffers(renderer_params);

    bool rebuild_cmd_buffer = false;
    while (!glfwWindowShouldClose(_context->window->get_window())) {
        glfwPollEvents();
        rebuild_cmd_buffer = fr::GLFWWindow::process_input(_context->window->get_window(), renderer_params.polygon_mode);

        if (rebuild_cmd_buffer) {
            renderer.build_command_buffers(renderer_params);
        }

        auto res = renderer.draw();
        if (!res) {
            _vulkan_builder->recreate_swap_chain();
            renderer.build_command_buffers(renderer_params);
            renderer.draw();
        }

        // Update MVP descriptor set
        fr::ViewProj::update(_context->allocator, _context->descriptor.uniform_buffer.allocation, _context->swap_chain_dimensions);
    }
}
