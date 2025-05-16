#include "drawing/descriptor_set.h"
#include "utils/error.h"

namespace fr {
    DescriptorSet::DescriptorSet(std::shared_ptr<VkContext>& context)
        : _context(context)
    { }

    void DescriptorSet::create_descriptor_sets(DescriptorCore& core, VkDescriptorType type, VkShaderStageFlags flags, std::uint32_t size, std::uint32_t binding) {
        const std::uint32_t n_descriptors = core.buffers.size();

        // Create descriptor layout for each descriptor set
        create_descriptor_layout(core.layout, type, flags, binding);
        auto layouts = std::vector<VkDescriptorSetLayout>(n_descriptors, core.layout);

        create_descriptor_pool(core.pool, type, n_descriptors);

        VkDescriptorSetAllocateInfo set_alloc_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = core.pool,
            .descriptorSetCount = n_descriptors,
            .pSetLayouts = layouts.data()
        };

        core.descriptors = {};
        core.descriptors.resize(n_descriptors);
        validate(
            vkAllocateDescriptorSets(_context->device, &set_alloc_info, core.descriptors.data()),
            "Failed to create descriptor set."
        );

        for (int i = 0; i < n_descriptors; ++i) {
            create_descriptor_set(core.descriptors[i], core.buffers[i].buffer, type, binding, size);
        }
    }

    void DescriptorSet::create_descriptor_layout(VkDescriptorSetLayout& descriptor_set_layout, VkDescriptorType type, VkShaderStageFlags flags, std::uint32_t binding) {
        VkDescriptorSetLayoutBinding set_layout_binding {
            .binding         = binding,
            .descriptorType  = type,
            .descriptorCount = 1,
            .stageFlags      = flags
        };

        VkDescriptorSetLayoutCreateInfo set_create_info {
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings    = &set_layout_binding
        };

        validate(
            vkCreateDescriptorSetLayout(_context->device, &set_create_info, nullptr, &descriptor_set_layout),
            "Failed to create descriptor set layout."
        );
    }

    /// Creates a descriptor pool which can be used to allocate descriptor sets.
    void DescriptorSet::create_descriptor_pool(VkDescriptorPool& descriptor_pool, const VkDescriptorType type, const std::uint32_t pool_size) {
        VkDescriptorPoolSize descriptor_pool_size {
            .type            = type,
            .descriptorCount = pool_size
        };

        VkDescriptorPoolCreateInfo pool_info {
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets       = pool_size,  // Maximum number of descriptor sets that can be allocated from the pool.
            .poolSizeCount = 1,
            .pPoolSizes    = &descriptor_pool_size,
        };

        validate(
            vkCreateDescriptorPool(_context->device, &pool_info, nullptr, &descriptor_pool),
            "Failed to create descriptor pool."
        );
    }

    void DescriptorSet::create_descriptor_set(VkDescriptorSet& descriptor_set, VkBuffer& buffer, VkDescriptorType type, std::uint32_t binding, std::uint32_t size) {
        VkDescriptorBufferInfo description {
            .buffer = buffer,
            .offset = 0,
            .range = size
        };

        VkWriteDescriptorSet write_descriptor {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptor_set,
            .dstBinding = binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = type,
            .pBufferInfo = &description
        };
        vkUpdateDescriptorSets(_context->device, 1, &write_descriptor, 0, nullptr);
    }
}
