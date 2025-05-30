#pragma once

#include <stdexcept>
#include <glm/glm.hpp>

namespace fr {
    struct VertexInfo {
        std::vector<VkVertexInputBindingDescription> binding_description {};
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions {};
        VkPipelineVertexInputStateCreateInfo vertex_info {};

        void generate_vertex_info() {
            if (binding_description.empty() || attribute_descriptions.empty()) {
                throw std::runtime_error("Unable to generate vertex info because binding and/or attribute descriptions are empty.");
            }

            vertex_info = VkPipelineVertexInputStateCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = static_cast<std::uint32_t>(binding_description.size()),
                .pVertexBindingDescriptions = binding_description.data(),
                .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attribute_descriptions.size()),
                .pVertexAttributeDescriptions = attribute_descriptions.data()
            };
        }

        void add_attribute_description(const VkFormat& format, const std::uint32_t& offset, const std::uint32_t binding) {
            attribute_descriptions.push_back(
                VkVertexInputAttributeDescription {
                    .location = static_cast<std::uint32_t>(attribute_descriptions.size()),
                    .binding = binding,
                    .format = format,
                    .offset = offset
                }
            );
        }

        void add_attribute_description(const glm::vec2& member, const std::uint32_t offset, const std::uint32_t binding) {
            add_attribute_description(VK_FORMAT_R32G32_SFLOAT, offset, binding);
        }

        void add_attribute_description(const glm::vec3& member, const std::uint32_t offset, const std::uint32_t binding) {
            add_attribute_description(VK_FORMAT_R32G32B32_SFLOAT, offset, binding);
        }

        void add_attribute_description(const std::uint32_t& member, const std::uint32_t offset, const std::uint32_t binding) {
            add_attribute_description(VK_FORMAT_R32_UINT, offset, binding);
        }

        void add_attribute_description(const glm::mat4& member, const std::uint32_t offset, const std::uint32_t binding) {
            add_attribute_description(VK_FORMAT_R32G32B32A32_SFLOAT, offset + sizeof(glm::vec4) * 0, binding);
            add_attribute_description(VK_FORMAT_R32G32B32A32_SFLOAT, offset + sizeof(glm::vec4) * 1, binding);
            add_attribute_description(VK_FORMAT_R32G32B32A32_SFLOAT, offset + sizeof(glm::vec4) * 2, binding);
            add_attribute_description(VK_FORMAT_R32G32B32A32_SFLOAT, offset + sizeof(glm::vec4) * 3, binding);
        }

        void add_binding_description(const std::uint32_t stride, const std::uint32_t binding, VkVertexInputRate input_rate) {
            binding_description.push_back(
                VkVertexInputBindingDescription {
                    .binding = binding,
                    .stride = stride,
                    .inputRate = input_rate
                }
            );
        }
    };
}