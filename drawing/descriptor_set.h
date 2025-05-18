#pragma once

#include "builders/vulkan_structures.h"

namespace fr {
    struct DescriptorInfo {
        VkDescriptorType type;
        VkShaderStageFlags flags;
        std::uint32_t binding;
        std::size_t size;
        VkDescriptorBufferInfo buffer_info;

        explicit DescriptorInfo(VkDescriptorType type_in, VkShaderStageFlags flags_in, std::uint32_t binding_in, std::size_t size_in, VkBuffer buffer, VkDeviceSize offset)
            : type(type_in)
            , flags(flags_in)
            , binding(binding_in)
            , size(size_in)
        {
            create_buffer_info(buffer, offset, size);
        }

        void create_buffer_info(VkBuffer buffer, const VkDeviceSize offset, const std::size_t size) {
            buffer_info = VkDescriptorBufferInfo {
                .buffer = buffer,
                .offset = offset,
                .range = size
            };
        }
    };

    class DescriptorSet {
    public:
        explicit DescriptorSet(std::shared_ptr<VkContext>& context);

        void create_descriptor_sets(DescriptorCore& core, std::vector<DescriptorInfo>& info);

        VkDescriptorSetLayoutBinding create_descriptor_layout(VkDescriptorType type, VkShaderStageFlags flags, std::uint32_t binding);

        void set_descriptor_layout(VkDescriptorSetLayout& descriptor_set_layout, std::vector<VkDescriptorSetLayoutBinding>& layout_bindings);

        VkDescriptorPoolSize create_descriptor_pool(VkDescriptorType type, std::uint32_t pool_size);

        void set_descriptor_pool(VkDescriptorPool& descriptor_pool, std::vector<VkDescriptorPoolSize>& pool_sizes, std::uint32_t max_sets);

        VkWriteDescriptorSet create_descriptor_set(VkDescriptorSet& descriptor_set, VkDescriptorBufferInfo* buffer_info, VkDescriptorType type, std::uint32_t binding);

    private:
        std::shared_ptr<VkContext> _context;
    };
}