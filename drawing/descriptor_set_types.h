#pragma once

#include "builders/vulkan_structures.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fr {
    struct ViewProj {
        glm::mat4 view;
        glm::mat4 proj;

        /// Updates the VP model with the Camera's perspective
        static void update(VmaAllocator allocator, VmaAllocation allocation, const SwapChainDimensions& swap_chain_dimensions) {
            auto vp = ViewProj {
                .view     = Camera::get_camera_view(),
                .proj     = glm::perspective(glm::radians(45.0f), static_cast<float>(swap_chain_dimensions.width) / static_cast<float>(swap_chain_dimensions.height), 0.1f, 2000.0f),
            };
            vp.proj[1][1] *= -1;

            vmaCopyMemoryToAllocation(allocator, &vp, allocation, 0, sizeof(vp));
        }
    };

    struct StorageBufferInfo {
        std::uint32_t size;
        std::uint32_t instance_size;
        float d;  // Distance between height indexes
    };

    struct FloatArray {
        std::vector<float> data;

        explicit FloatArray(const std::vector<float>& data_in)
            : data(data_in)
        { }

        [[nodiscard]] StorageBufferInfo get_storage_buffer_info(const std::uint32_t instance_size_in, float d_in) const {
            return StorageBufferInfo {
                .size = static_cast<std::uint32_t>(data.size()),
                .instance_size = instance_size_in,
                .d = d_in
            };
        }

        [[nodiscard]] std::uint32_t size() const {
            return sizeof(std::uint32_t) + sizeof(float) * data.size();
        }
    };
}