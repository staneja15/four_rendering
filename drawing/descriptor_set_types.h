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

    namespace dynamic {
        /// Creates a line of model matrices along the x-axis, intended for use with dynamic buffers
        struct Model {
            glm::mat4* model = nullptr;

            static void update(void* mapped_data, const std::size_t dynamic_alignment, const std::size_t buffer_size, const std::size_t n_components) {
                if (buffer_size % dynamic_alignment != 0) {
                    throw std::runtime_error("Buffer size must be a multiple of dynamic alignment.");
                }

                // Allocate and align the data
                void* data = nullptr;
                posix_memalign(&data, dynamic_alignment, buffer_size);

                auto [model] = Model {};
                model = static_cast<glm::mat4*>(data);

                for (int i = 0; i < n_components; ++i) {
                    auto model_mat = reinterpret_cast<glm::mat4*>(reinterpret_cast<uint64_t>(model) + (i * dynamic_alignment));
                    *model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f * static_cast<float>(i), 0.0f, 0.0f));
                }

                memcpy(mapped_data, model, buffer_size);
            }
        };
    }
}