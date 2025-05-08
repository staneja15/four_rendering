/*
 *  brief: The goal of the sample application is to show the basic tools required
 *   to set up an application using four_renderer.
 */

#pragma once

#include "builders/vulkan_builder.h"

class SampleApplication {
public:
    SampleApplication();

    void init();

    void run();

private:
    std::unique_ptr<fr::VulkanBuilder> _vulkan_builder;
    std::shared_ptr<VkContext> _context;
};