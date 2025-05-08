#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>

namespace fr {
    enum class graphics_api {
        Vulkan = 0,
        OpenGL = 1
    };

    namespace system {
        #ifdef NDEBUG
            constexpr bool enable_validation_layers = false;
        #else
            constexpr bool enable_validation_layers = true;
        #endif
    }

    namespace vulkan {
        const std::vector<const char*> validation_layers = {
            "VK_LAYER_KHRONOS_validation"
        };

        const std::vector<const char*> device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            "VK_EXT_extended_dynamic_state3"
        };
    }
}