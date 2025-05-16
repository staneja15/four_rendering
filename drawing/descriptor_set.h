#pragma once

#include "builders/vulkan_structures.h"

namespace fr {
    class DescriptorSet {
    public:
        explicit DescriptorSet(std::shared_ptr<VkContext>& context);

        void create_descriptor_sets(DescriptorCore& core, VkDescriptorType type, VkShaderStageFlags flags, std::uint32_t size, std::uint32_t binding);

        void create_descriptor_layout(VkDescriptorSetLayout& descriptor_set_layout, VkDescriptorType type, VkShaderStageFlags flags, std::uint32_t binding);

        void create_descriptor_pool(VkDescriptorPool& descriptor_pool, VkDescriptorType type, std::uint32_t pool_size);

        void create_descriptor_set(VkDescriptorSet& descriptor_set, VkBuffer& buffer, VkDescriptorType type, std::uint32_t binding, std::uint32_t size);

    private:
        std::shared_ptr<VkContext> _context;
    };
}