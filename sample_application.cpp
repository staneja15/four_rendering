#include "sample_application.h"
#include "drawing/graphics_pipeline.h"
#include "drawing/buffer.h"
#include "drawing/renderer.h"
#include "drawing/descriptor_set.h"
#include "drawing/descriptor_set_types.h"
#include "shaders/shader.h"

SampleApplication::SampleApplication()
    : _vulkan_builder(std::make_unique<fr::VulkanBuilder>())
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

    auto buffer = fr::Buffer(_context);
    buffer.create_buffer(_context->vertex_buffer,  vertices.data(), sizeof(vertices[0]) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true);
    buffer.create_buffer(_context->indices_buffer, indices.data(),  sizeof(indices[0])  * indices.size(),  VK_BUFFER_USAGE_INDEX_BUFFER_BIT,  true);

    // Create descriptor sets
    const std::uint32_t n_descriptors = _context->per_frame.size();
    _context->mvp.buffers.reserve(n_descriptors);

    auto data = fr::MVP {};
    for (int i = 0; i < n_descriptors; ++i) {
        // Initialise a descriptor set for each frame
        _context->mvp.buffers.emplace_back(&_context->device);
        buffer.create_buffer(_context->mvp.buffers[i], &data, sizeof(fr::MVP), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, false);
    }

    auto descriptor_set = fr::DescriptorSet(_context);
    descriptor_set.create_descriptor_sets(_context->mvp, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(fr::MVP), 0);
}

void SampleApplication::run() {
    // Initialise the renderer
    auto renderer = fr::Renderer(_context);

    while (!glfwWindowShouldClose(_context->window->get_window())) {
        glfwPollEvents();
        fr::GLFWWindow::process_input(_context->window->get_window());

        auto res = renderer.record_command_buffer(vertices.size());
        if (!res) {
            _vulkan_builder->recreate_swap_chain();
            renderer.record_command_buffer(vertices.size());
        }
    }
}
