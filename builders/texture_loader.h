#pragma once

#include "vulkan_structures.h"

#include <filesystem>

namespace fr {
    struct TextureInfo {
        std::size_t size;
        VkImage     image;
        VkImageView view;
        VkSampler   sampler;
        VkDescriptorImageInfo image_info;
    };

    class Texture {
    public:
        Texture(const std::shared_ptr<VkContext>& context);

        ~Texture();

        void load(const std::filesystem::path& path);

        TextureInfo get_info();

    private:
        std::shared_ptr<VkContext> _context;
        void*         _data;
        VkImageLayout _image_layout;
        VmaAllocation _allocation;
        uint32_t      _mip_levels;
        BufferCore    _staging_buffer;
        TextureInfo   _info;

        void _prepare_resources(std::uint32_t width, std::uint32_t height);

        void _copy_data(std::uint32_t width, std::uint32_t height);

        void _create_sampler();

        void _create_view();
    };
}
