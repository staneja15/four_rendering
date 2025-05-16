#pragma once

#include "builders/vulkan_structures.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fr {
    struct MVP {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;

        /// Updates the MVP model with the Camera's perspective
        static void update(void* mapped_data, const SwapChainDimensions& swap_chain_dimensions) {
            auto mvp = MVP {
                .model = glm::mat4(1.0f),
                .view  = Camera::get_camera_view(),
                .proj  = glm::perspective(glm::radians(45.0f), swap_chain_dimensions.width / (float) swap_chain_dimensions.height, 0.1f, 100.0f)
            };
            mvp.proj[1][1] *= -1;

            memcpy(mapped_data, &mvp, sizeof(mvp));
        }
    };
}