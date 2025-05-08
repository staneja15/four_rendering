#pragma once

#include <window/GLFW_window.h>

#include <vector>
#include <memory>

#include <vulkan/vulkan.h>

struct SurfaceProperties {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR       format;
};

struct Extensions {
	PFN_vkCmdSetPolygonModeEXT polygon_mode = VK_NULL_HANDLE;
};

struct SwapChainDimensions {
	/// Width of the swap chain.
	uint32_t width = 0;

	/// Height of the swap chain.
	uint32_t height = 0;

	/// Pixel format of the swap chain.
	VkFormat format = VK_FORMAT_UNDEFINED;
};

struct PerFrame {
	VkFence         queue_submit_fence           = VK_NULL_HANDLE;
	VkCommandPool   primary_command_pool         = VK_NULL_HANDLE;
	VkCommandBuffer primary_command_buffer       = VK_NULL_HANDLE;
	VkSemaphore     swap_chain_acquire_semaphore = VK_NULL_HANDLE;
	VkSemaphore     swap_chain_release_semaphore = VK_NULL_HANDLE;
};

struct VkContext {
	/// The additional vulkan extensions required by the renderer
	Extensions extensions = {};

	/// The GLFW application window
	std::shared_ptr<fr::GLFWWindow> window = VK_NULL_HANDLE;

	/// The Vulkan instance.
    VkInstance instance = VK_NULL_HANDLE;

    /// The Vulkan physical device.
    VkPhysicalDevice gpu = VK_NULL_HANDLE;

    /// The Vulkan device.
    VkDevice device = VK_NULL_HANDLE;

    /// The Vulkan device queue.
    VkQueue queue = VK_NULL_HANDLE;

    /// The swap chain.
    VkSwapchainKHR swap_chain = VK_NULL_HANDLE;

    /// The swap chain dimensions.
    SwapChainDimensions swap_chain_dimensions;

    /// The surface we will render to.
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    /// The queue family index where graphics work will be submitted.
    int32_t graphics_queue_index = -1;

    /// The image view for each swap chain image.
    std::vector<VkImageView> swap_chain_image_views;

    /// The handles to the images in the swap chain.
    std::vector<VkImage> swap_chain_images;

    /// The graphics pipeline.
    VkPipeline pipeline = VK_NULL_HANDLE;

    /**
     * The pipeline layout for resources.
     * Not used in this sample, but we still need to provide a dummy one.
     */
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

    /// The debug utility messenger callback.
    VkDebugUtilsMessengerEXT debug_callback = VK_NULL_HANDLE;

    /// A set of semaphores that can be reused.
    std::vector<VkSemaphore> recycled_semaphores;

    /// A set of per-frame data.
    std::vector<PerFrame> per_frame;

    /// The Vulkan buffer object that holds the vertex data for the triangle.
    VkBuffer vertex_buffer = VK_NULL_HANDLE;

    /// The device memory allocated for the vertex buffer.
    VkDeviceMemory vertex_buffer_memory = VK_NULL_HANDLE;

	void teardown_per_frame(PerFrame& per_frame) {
		if (per_frame.queue_submit_fence != VK_NULL_HANDLE) {
			vkDestroyFence(device, per_frame.queue_submit_fence, nullptr);

			per_frame.queue_submit_fence = VK_NULL_HANDLE;
		}

		if (per_frame.primary_command_buffer != VK_NULL_HANDLE) {
			vkFreeCommandBuffers(device, per_frame.primary_command_pool, 1, &per_frame.primary_command_buffer);

			per_frame.primary_command_buffer = VK_NULL_HANDLE;
		}

		if (per_frame.primary_command_pool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device, per_frame.primary_command_pool, nullptr);

			per_frame.primary_command_pool = VK_NULL_HANDLE;
		}

		if (per_frame.swap_chain_acquire_semaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, per_frame.swap_chain_acquire_semaphore, nullptr);

			per_frame.swap_chain_acquire_semaphore = VK_NULL_HANDLE;
		}

		if (per_frame.swap_chain_release_semaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, per_frame.swap_chain_release_semaphore, nullptr);

			per_frame.swap_chain_release_semaphore = VK_NULL_HANDLE;
		}
	}

	~VkContext() {
		// Don't release anything until the GPU is completely idle
		if (device != VK_NULL_HANDLE)
			vkDeviceWaitIdle(device);

		// Free device attachments
		for (auto& semaphore : recycled_semaphores) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}
		recycled_semaphores.clear();

		if (vertex_buffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, vertex_buffer, nullptr);
			vertex_buffer = VK_NULL_HANDLE;
		}

		if (vertex_buffer_memory != VK_NULL_HANDLE) {
			vkFreeMemory(device, vertex_buffer_memory, nullptr);
			vertex_buffer_memory = VK_NULL_HANDLE;
		}

		if (pipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(device, pipeline, nullptr);
			pipeline = VK_NULL_HANDLE;
		}

		if (pipeline_layout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
			pipeline_layout = VK_NULL_HANDLE;
		}

		for (auto& image_view : swap_chain_image_views) {
			vkDestroyImageView(device, image_view, nullptr);
		}

		for (auto& per_frame : per_frame) {
			teardown_per_frame(per_frame);
		}
		per_frame.clear();

		if (swap_chain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(device, swap_chain, nullptr);
			swap_chain = VK_NULL_HANDLE;
		}

		if (device != VK_NULL_HANDLE) {
			vkDestroyDevice(device, nullptr);
			device = VK_NULL_HANDLE;
		}

		// Free instance attachments
		if (surface != VK_NULL_HANDLE) {
			vkDestroySurfaceKHR(instance, surface, nullptr);
			surface = VK_NULL_HANDLE;
		}

		if (instance != VK_NULL_HANDLE) {
			vkDestroyInstance(instance, nullptr);
			instance = VK_NULL_HANDLE;
		}
	}
};