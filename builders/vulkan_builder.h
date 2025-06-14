#pragma once

#define VULKAN_VERSION VK_API_VERSION_1_3

#include "vulkan_structures.h"

#include <memory>
#include <filesystem>

#include <vulkan/vulkan_core.h>

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

        void _create_memory_allocator();

        void _load_device_extensions();

        void _init_per_frame(PerFrame& per_frame);

        void _create_swap_chain();

        void _create_depth_resources();

        std::uint32_t _find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const;
    };
} // namespace fr