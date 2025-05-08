#include "sample_application.h"
#include "drawing/graphics_pipeline.h"
#include "drawing/vertex_types.h"
#include "drawing/vertex_buffer.h"
#include "drawing/renderer.h"
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

    // Create the graphics pipeline
    auto terrain_pipeline = fr::GraphicsPipeline(_context);
    terrain_pipeline.create_pipeline(vertex_info, shader_stages);

    shader.destroy_shaders();  // shaders are now baked into the pipeline, we can now freely destroy them
}

void SampleApplication::run() {
    // Create vertex data
    auto vertices = std::vector<fr::HelloTriangleVertex> {
            {{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},        // Vertex 1: Red
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},         // Vertex 2: Green
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}         // Vertex 3: Blue
    };

    auto vertex_buffer = fr::VertexBuffer(_context);
    vertex_buffer.create_vertex_buffer(vertices.data(), sizeof(vertices[0]) * vertices.size());

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
