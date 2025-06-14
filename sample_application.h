/*
 *  brief: The goal of the sample application is to show the basic tools required
 *   to set up an application using four_renderer.
 */

#pragma once

#include "builders/vulkan_builder.h"
#include "drawing/vertex_types.h"
#include "builders/texture_loader.h"

inline auto vertices = std::vector<fr::HelloTriangleVertex> {
    {{0.5f, -0.5f},  {1.0f, 0.0f, 0.0f}, {1, 1}},        // Vertex 1: Red

    {{0.5f, 0.5f},   {0.0f, 1.0f, 0.0f}, {1, 0}},        // Vertex 2: Green
    {{-0.5f, 0.5f},  {0.0f, 0.0f, 1.0f}, {0, 0}},        // Vertex 3: Blue
    {{-0.5f, -0.5f}, {0.6f, 0.3f, 0.8f}, {0, 1}}         // Vertex 3: Pink
};

class SampleApplication {
public:
    SampleApplication();

    void init();

    void set_uniforms();

    void run();

private:
    std::unique_ptr<fr::VulkanBuilder> _vulkan_builder;
    std::shared_ptr<VkContext> _context;
    std::unique_ptr<fr::Texture> _texture;
};