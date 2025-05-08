#pragma once

#include "vulkan_structures.h"
#include "utils/global.h"
#include "window/GLFW_window.h"

#include <memory>

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace fr {
    static VKAPI_ATTR VkBool32 VKAPI_CALL _debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );

    class VulkanBuilder {
    public:
        VulkanBuilder();

        void prepare();

        void recreate_swap_chain();

        [[nodiscard]] std::shared_ptr<VkContext> get_context() const;

    private:
        std::shared_ptr<VkContext> _context;

        static bool _validate_extensions(
            const std::vector<const char *>& required,
            const std::vector<VkExtensionProperties>& available
        );

        static std::vector<const char*> _get_requested_layers();

        static std::vector<const char*> _get_required_extensions();

        static VkDebugUtilsMessengerCreateInfoEXT get_debug_info();

        void _create_instance();

        void _create_surface();

        void _create_device();

        void _load_device_extensions();

        void _init_per_frame(PerFrame& per_frame);

        void _create_swap_chain();
    };
} // namespace fr