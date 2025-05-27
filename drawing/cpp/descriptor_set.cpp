#include "drawing/descriptor_set.h"
#include "utils/error.h"

#include <unordered_map>

namespace fr {
    DescriptorSet::DescriptorSet(std::shared_ptr<VkContext>& context)
        : _context(context)
    { }

    void DescriptorSet::create_descriptor_sets(DescriptorCore& core, std::vector<DescriptorInfo>& info) {
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings = {};
        layout_bindings.reserve(info.size());

        std::vector<VkDescriptorPoolSize> pool_sizes = {};
        pool_sizes.reserve(info.size());

        std::vector<VkWriteDescriptorSet> write_descriptor_sets = {};
        write_descriptor_sets.reserve(info.size());


        std::unordered_map<VkDescriptorType, std::uint16_t> type_info = {};
        for (auto& [type, flags, binding, size, buffer_info] : info) {
            layout_bindings.emplace_back(create_descriptor_layout(type, flags, binding));

            type_info[type]++;  // Aggregate the type information
        }

        for (const auto& [type, count] : type_info) {
            pool_sizes.emplace_back(create_descriptor_pool(type, count));
        }

        set_descriptor_layout(core.layout, layout_bindings);
        set_descriptor_pool(core.pool, pool_sizes, info.size());

        VkDescriptorSetAllocateInfo set_alloc_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = core.pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &core.layout
        };

        validate(
            vkAllocateDescriptorSets(_context->device, &set_alloc_info, &core.descriptor),
            "Failed to create descriptor set."
        );

        for (auto& [type, flags, binding, size, buffer_info] : info) {
            write_descriptor_sets.emplace_back(create_descriptor_set(core.descriptor, &buffer_info, type, binding));
        }

        vkUpdateDescriptorSets(_context->device, write_descriptor_sets.size(), write_descriptor_sets.data(), 0, nullptr);
    }

    VkDescriptorSetLayoutBinding DescriptorSet::create_descriptor_layout(const VkDescriptorType type, const VkShaderStageFlags flags, const std::uint32_t binding) {
        return VkDescriptorSetLayoutBinding {
            .binding         = binding,
            .descriptorType  = type,
            .descriptorCount = 1,
            .stageFlags      = flags
        };
    }

    void DescriptorSet::set_descriptor_layout(VkDescriptorSetLayout& descriptor_set_layout, std::vector<VkDescriptorSetLayoutBinding>& layout_bindings) {
        VkDescriptorSetLayoutCreateInfo set_create_info {
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<std::uint32_t>(layout_bindings.size()),
            .pBindings    = layout_bindings.data()
        };

        validate(
            vkCreateDescriptorSetLayout(_context->device, &set_create_info, nullptr, &descriptor_set_layout),
            "Failed to create descriptor set layout."
        );
    }

    VkDescriptorPoolSize DescriptorSet::create_descriptor_pool(const VkDescriptorType type, const std::uint32_t pool_size) {
        return VkDescriptorPoolSize {
            .type            = type,
            .descriptorCount = pool_size
        };
    }

    void DescriptorSet::set_descriptor_pool(VkDescriptorPool& descriptor_pool, std::vector<VkDescriptorPoolSize>& pool_sizes, const std::uint32_t max_sets) {
        VkDescriptorPoolCreateInfo pool_info {
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets       = max_sets,  // Maximum number of descriptor sets that can be allocated from the pool.
            .poolSizeCount = 1,
            .pPoolSizes    = pool_sizes.data()
        };

        validate(
            vkCreateDescriptorPool(_context->device, &pool_info, nullptr, &descriptor_pool),
            "Failed to create descriptor pool."
        );
    }

    VkWriteDescriptorSet DescriptorSet::create_descriptor_set(VkDescriptorSet& descriptor_set, VkDescriptorBufferInfo* buffer_info, VkDescriptorType type, std::uint32_t binding) {
        VkWriteDescriptorSet write_descriptor {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptor_set,
            .dstBinding = binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = type,
            .pBufferInfo = buffer_info
        };

        return write_descriptor;
    }
}
