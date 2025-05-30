#pragma once

#include "builders/vulkan_structures.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fr {
    struct ViewProj {
        glm::mat4 view;
        glm::mat4 proj;

        /// Updates the VP model with the Camera's perspective
        static void update(void* mapped_data, const SwapChainDimensions& swap_chain_dimensions) {
            auto vp = ViewProj {
                .view  = Camera::get_camera_view(),
                .proj  = glm::perspective(glm::radians(45.0f), static_cast<float>(swap_chain_dimensions.width) / static_cast<float>(swap_chain_dimensions.height), 0.1f, 100.0f)
            };
            vp.proj[1][1] *= -1;

            memcpy(mapped_data, &vp, sizeof(vp));
        }
    };
}