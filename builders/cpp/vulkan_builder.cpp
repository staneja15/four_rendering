#include "builders/vulkan_builder.h"
#include "utils/error.h"

#include <iostream>

namespace fr {
    VulkanBuilder::VulkanBuilder()
        : _context(std::make_shared<VkContext>())
    {
        _context->window = std::make_shared<GLFWWindow>(800, 600);
    }

    void VulkanBuilder::prepare() {
        _context->window->init();

        _create_instance();
        _create_surface();
        _create_device();
        _load_device_extensions();
        _create_swap_chain();
    }

    void VulkanBuilder::recreate_swap_chain() {
        _create_swap_chain();
    }

    std::shared_ptr<VkContext> VulkanBuilder::get_context() const {
        return _context;
    }

    bool VulkanBuilder::_validate_extensions(const std::vector<const char *>& required, const std::vector<VkExtensionProperties>& available) {
        bool all_found = true;

        for (const auto* extension_name : required) {
            bool found = false;
            for (const auto& available_extension : available) {
                if (strcmp(available_extension.extensionName, extension_name) == 0) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                // Output an error message for the missing extension
                std::cout << "Error: Required extension not found: " << extension_name << "\n";
                all_found = false;
            }
        }

        return all_found;
    }

    VkBool32 _debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    std::vector<const char*> VulkanBuilder::_get_requested_layers() {
        std::vector<const char*> requested_layers = {};
        if (system::enable_validation_layers)
            requested_layers = vulkan::validation_layers;


        return requested_layers;
    }

    std::vector<const char*> VulkanBuilder::_get_required_extensions() {
        std::uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (system::enable_validation_layers) {
            extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    VkDebugUtilsMessengerCreateInfoEXT VulkanBuilder::get_debug_info() {
        VkDebugUtilsMessengerCreateInfoEXT debug_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        if (system::enable_validation_layers) {
            debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            debug_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
            debug_info.pfnUserCallback = _debug_callback;
        }

        return debug_info;
    }

    void VulkanBuilder::_create_instance() {
        VkApplicationInfo app {
            .sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Four Rendering",
            .pEngineName      = "Four",
            .apiVersion       = VK_MAKE_VERSION(1, 3, 0)
        };

        std::vector<const char*> requested_layers = _get_requested_layers();
        std::vector<const char*> required_extensions = _get_required_extensions();
        VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = get_debug_info();

        VkInstanceCreateInfo instance_info {
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext                   = &debug_messenger_create_info,
            .pApplicationInfo        = &app,
            .enabledLayerCount       = static_cast<std::uint32_t>(requested_layers.size()),
            .ppEnabledLayerNames     = requested_layers.data(),
            .enabledExtensionCount   = static_cast<std::uint32_t>(required_extensions.size()),
            .ppEnabledExtensionNames = required_extensions.data()
        };

        validate(vkCreateInstance(&instance_info, nullptr, &_context->instance), "Failed to create vulkan instance.");
    }

    void VulkanBuilder::_create_surface() {
        if (_context->instance == VK_NULL_HANDLE)
            throw std::runtime_error("Unable to create surface because instance is not initialised.");

        validate(
            glfwCreateWindowSurface(_context->instance, _context->window->get_window(), NULL, &_context->surface),
            "Failed to create surface."
        );

        _context->swap_chain_dimensions.width = _context->window->width();
        _context->swap_chain_dimensions.height = _context->window->height();
    }

    void VulkanBuilder::_create_device() {
        // Find a physical device on the machine that supports Vulkan 1.3
        std::uint32_t gpu_count = 0;
        vkEnumeratePhysicalDevices(_context->instance, &gpu_count, nullptr);

        if (gpu_count < 1)
            throw std::runtime_error("No Physical Devices were found.");

        std::vector<VkPhysicalDevice> gpus(gpu_count);
        vkEnumeratePhysicalDevices(_context->instance, &gpu_count, gpus.data());

        for (const auto& physical_device: gpus) {
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(physical_device, &device_properties);

            if (device_properties.apiVersion < VK_API_VERSION_1_3) {
                std::cout << "Skipping physical device: does not support Vulkan 1.3\n";
                continue;
            }

            // Find a queue family that supports graphics and presentation
            std::uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

            std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties.data());

            for (std::uint32_t i = 0; i < queue_family_count; ++i) {
                VkBool32 supports_present = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, _context->surface, &supports_present);

                if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supports_present) {
                    // Successfully found a queue family that supports both graphics and present!
                    _context->graphics_queue_index = i;
                    break;
                }
            }

            if (_context->graphics_queue_index >= 0) {
                // appropriate queue was found: assign the physical device to the gpu context
                _context->gpu = physical_device;
                break;
            }
        }

        if (_context->graphics_queue_index < 0)
            throw std::runtime_error("Failed to find a suitable GPU with Vulkan 1.3 support");

        // Get the required extensions for the physical device
        std::uint32_t device_extension_count = 0;
        vkEnumerateDeviceExtensionProperties(_context->gpu, nullptr, &device_extension_count, nullptr);

        std::vector<VkExtensionProperties> device_extensions(device_extension_count);
        vkEnumerateDeviceExtensionProperties(_context->gpu, nullptr, &device_extension_count, device_extensions.data());

        std::vector<const char *> required_device_extensions = fr::vulkan::device_extensions;
        if (!_validate_extensions(required_device_extensions, device_extensions))
            throw std::runtime_error("Failed to find all required device extensions on the selected physical device.");

        // Query for Vulkan 1.3 features
        VkPhysicalDeviceFeatures2 query_device_features2 { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
        VkPhysicalDeviceVulkan13Features query_vulkan13_features { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT query_extended_dynamic_state_features { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
        query_device_features2.pNext = &query_vulkan13_features;
        query_vulkan13_features.pNext = &query_extended_dynamic_state_features;

        vkGetPhysicalDeviceFeatures2(_context->gpu, &query_device_features2);

        if (!query_vulkan13_features.dynamicRendering)
            throw std::runtime_error("Dynamic Rendering feature is not supported.");

        if (!query_vulkan13_features.synchronization2)
            throw std::runtime_error("Synchronization2 feature is not supported.");

        if (!query_extended_dynamic_state_features.extendedDynamicState)
            throw std::runtime_error("Extended Dynamic State is not supported.");

        // Enable the specific Vulkan 1.3 features that we are going to use
        VkPhysicalDeviceExtendedDynamicState3FeaturesEXT enable_extended_dynamic_state_3_features {
            .sType                            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
            .extendedDynamicState3PolygonMode = VK_TRUE
        };

        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT enable_extended_dynamic_state_features {
            .sType                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
            .pNext                = &enable_extended_dynamic_state_3_features,
            .extendedDynamicState = VK_TRUE
        };

        VkPhysicalDeviceVulkan13Features enable_vulkan13_features {
            .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext            = &enable_extended_dynamic_state_features,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE
        };

        VkPhysicalDeviceFeatures2 enable_device_features2 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &enable_vulkan13_features
        };

        // Create the logical device
        float queue_priority = 1.0f;

        VkDeviceQueueCreateInfo queue_info {
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = static_cast<uint32_t>(_context->graphics_queue_index),
            .queueCount       = 1,
            .pQueuePriorities = &queue_priority
        };

        VkDeviceCreateInfo device_info {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext                   = &enable_device_features2,
            .queueCreateInfoCount    = 1,
            .pQueueCreateInfos       = &queue_info,
            .enabledExtensionCount   = static_cast<std::uint32_t>(required_device_extensions.size()),
            .ppEnabledExtensionNames = required_device_extensions.data()
        };

        validate(
            vkCreateDevice(_context->gpu, &device_info, nullptr, &_context->device),
            "Failed to create logical device."
        );

        vkGetDeviceQueue(_context->device, _context->graphics_queue_index, 0, &_context->queue);
    }

    void VulkanBuilder::_load_device_extensions() {
        // Allows us to dynamically set the polygon mode during render time.
        _context->extensions.polygon_mode = reinterpret_cast<PFN_vkCmdSetPolygonModeEXT>(
            vkGetDeviceProcAddr(_context->device, "vkCmdSetPolygonModeEXT")
        );
    }

    void VulkanBuilder::_init_per_frame(PerFrame& per_frame) {
        VkFenceCreateInfo fence_info {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
        validate(
            vkCreateFence(_context->device, &fence_info, nullptr, &per_frame.queue_submit_fence),
            "Failed to create fence."
        );

        VkCommandPoolCreateInfo cmd_pool_info {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = static_cast<uint32_t>(_context->graphics_queue_index)
        };
        validate(
            vkCreateCommandPool(_context->device, &cmd_pool_info, nullptr, &per_frame.primary_command_pool),
            "Failed to create command pool."
        );

        VkCommandBufferAllocateInfo cmd_buf_info{
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = per_frame.primary_command_pool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        validate(
            vkAllocateCommandBuffers(_context->device, &cmd_buf_info, &per_frame.primary_command_buffer),
            "Failed to allocate command buffers."
        );
    }

    void VulkanBuilder::_create_swap_chain() {
        SurfaceProperties surface_properties {};

        // Get the swap chain capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_context->gpu, _context->surface, &surface_properties.capabilities);

        // Select a surface format
        uint32_t surface_format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(_context->gpu, _context->surface, &surface_format_count, nullptr);
        assert(0 < surface_format_count);

        std::vector<VkSurfaceFormatKHR> supported_surface_formats(surface_format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_context->gpu, _context->surface, &surface_format_count, supported_surface_formats.data());

        for (const auto& available_surface_formats : supported_surface_formats) {
            if (available_surface_formats.format == VK_FORMAT_B8G8R8A8_SRGB && available_surface_formats.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surface_properties.format = available_surface_formats;
            }
        }

        // Get swap chain dimensions
        VkExtent2D swap_chain_size;
        if (surface_properties.capabilities.currentExtent.width == 0xFFFFFFFF) {
            // This occurs with very high resolution displays
            swap_chain_size.width  = _context->swap_chain_dimensions.width;
            swap_chain_size.height = _context->swap_chain_dimensions.height;
        } else {
            swap_chain_size = surface_properties.capabilities.currentExtent;
        }

        // FIFO must be supported by all implementations.
        VkPresentModeKHR swap_chain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

        // Determine the number of VkImage's to use in the swapchain.
        // Ideally, we desire to own 1 image at a time, the rest of the images can
        // either be rendered to and/or being queued up for display.
        uint32_t desired_swap_chain_images = surface_properties.capabilities.minImageCount + 1;
        if ((surface_properties.capabilities.maxImageCount > 0) && (desired_swap_chain_images > surface_properties.capabilities.maxImageCount)) {
            // Application must settle for fewer images than desired.
            desired_swap_chain_images = surface_properties.capabilities.maxImageCount;
        }

        // Find a suitable surface transform
        VkSurfaceTransformFlagBitsKHR pre_transform;
        if (surface_properties.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
            // Specifies that the image that is being presented is not transformed
            pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        } else {
            pre_transform = surface_properties.capabilities.currentTransform;
        }

        // one bitmask needs to be set according to the priority of presentation engine
        VkCompositeAlphaFlagBitsKHR composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        if (surface_properties.capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
            composite = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        }
        else if (surface_properties.capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
            composite = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        }
        else if (surface_properties.capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
            composite = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
        }
        else if (surface_properties.capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
            composite = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
        }

        VkSwapchainKHR old_swap_chain = _context->swap_chain;

        // Create the swap chain
        VkSwapchainCreateInfoKHR swap_chain_info {
            .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface          = _context->surface,                          // The surface onto which images will be presented
            .minImageCount    = desired_swap_chain_images,                  // Minimum number of images in the swapchain (number of buffers)
            .imageFormat      = surface_properties.format.format,           // Format of the swapchain images (e.g., VK_FORMAT_B8G8R8A8_SRGB)
            .imageColorSpace  = surface_properties.format.colorSpace,       // Color space of the images (e.g., VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            .imageExtent      = swap_chain_size,                            // Resolution of the swapchain images (width and height)
            .imageArrayLayers = 1,                                          // Number of layers in each image (usually 1 unless stereoscopic)
            .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,        // How the images will be used (as color attachments)
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,                  // Access mode of the images (exclusive to one queue family)
            .preTransform     = pre_transform,                              // Transform to apply to images (e.g., rotation)
            .compositeAlpha   = composite,                                  // Alpha blending to apply (e.g., opaque, pre-multiplied)
            .presentMode      = swap_chain_present_mode,                    // Presentation mode (e.g., vsync settings)
            .clipped          = true,                                       // Whether to clip obscured pixels (improves performance)
            .oldSwapchain     = old_swap_chain                              // Handle to the old swapchain, if replacing an existing one
        };

        validate(
            vkCreateSwapchainKHR(_context->device, &swap_chain_info, nullptr, &_context->swap_chain),
            "Failed to create Swap Chain."
        );

        // Clean up old swap chain if it exists: This can happen when we create new swap chains for resizing
        if (old_swap_chain != VK_NULL_HANDLE) {
            for (const VkImageView image_view : _context->swap_chain_image_views) {
                vkDestroyImageView(_context->device, image_view, nullptr);
            }

            for (auto& per_frame : _context->per_frame) {
                _context->teardown_per_frame(per_frame);
            }

            _context->swap_chain_image_views.clear();

            vkDestroySwapchainKHR(_context->device, old_swap_chain, nullptr);
        }

        // Get swap chain images
        _context->swap_chain_dimensions = {swap_chain_size.width, swap_chain_size.height, surface_properties.format.format};

        std::uint32_t image_count;
        vkGetSwapchainImagesKHR(_context->device, _context->swap_chain, &image_count, nullptr);

        std::vector<VkImage> swap_chain_images(image_count);
        vkGetSwapchainImagesKHR(_context->device, _context->swap_chain, &image_count, swap_chain_images.data());
        _context->swap_chain_images = swap_chain_images;

        _context->per_frame.clear();
        _context->per_frame.resize(image_count);

        for (size_t i = 0; i < image_count; ++i) {
            // Initialise the per frame structure
            _init_per_frame(_context->per_frame[i]);

            // Create an image view which we can render into
            VkImageViewCreateInfo view_info {
                .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext            = nullptr,
                .flags            = 0,
                .image            = swap_chain_images[i],
                .viewType         = VK_IMAGE_VIEW_TYPE_2D,
                .format           = _context->swap_chain_dimensions.format,
                .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
            };

            VkImageView image_view;
            validate(
                vkCreateImageView(_context->device, &view_info, nullptr, &image_view),
                "Failed to create image view."
            );
            _context->swap_chain_image_views.push_back(image_view);
        }

    }

}  // namespace fr
