#define STB_IMAGE_IMPLEMENTATION

#include "texture_loader.h"
#include "utils/buffer_utils.h"
#include "utils/error.h"
#include "utils/scoped_command_buffer.h"
#include "utils/image_utils.h"

#include "stb/stb_image.h"

namespace fr {
    Texture::Texture(const std::shared_ptr<VkContext>& context)
        : _staging_buffer(&context->device, &context->allocator)
        , _context(context)
    { }

    Texture::~Texture() {
        vkDeviceWaitIdle(_context->device);
        vkDestroyImageView(_context->device, _info.view, nullptr);
        vkDestroySampler(_context->device, _info.sampler, nullptr);
        vkDestroyImage(_context->device, _info.image, nullptr);

        _staging_buffer.destroy();
        vmaFreeMemory(_context->allocator, _allocation);
    }

    void Texture::load(const std::filesystem::path& path) {
        if (!exists(path)) {
            throw std::runtime_error("Texture image path was not found.");
        }

        // Load the texture
        int width, height, channels;
        _data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        _info.size = width * height * 4;

        // Allocate memory and create required resources for texture loading
        _prepare_resources(width, height);

        // Copy texture data from the staging buffer to the image in GPU
        _copy_data(width, height);

        _create_sampler();

        _create_view();
    }

    TextureInfo Texture::get_info() {
        _info.image_info = {
            .sampler     = _info.sampler,
            .imageView   = _info.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        return _info;
    }

    void Texture::_prepare_resources(const std::uint32_t width, const std::uint32_t height) {
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

        // Create the texture image
        VkImageCreateInfo image_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = {
                .width = width,
                .height = height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VmaAllocationCreateInfo alloc_create_info {
            .usage = VMA_MEMORY_USAGE_GPU_ONLY
        };

        VmaAllocationInfo alloc_info;
        validate(
            vmaCreateImage(_context->allocator, &image_info, &alloc_create_info, &_info.image, &_allocation, &alloc_info),
            "Failed to create image."
        );

        // Create the staging buffer
        auto buffer_utils = BufferUtils(_context);
        buffer_utils.create_staging_buffer(_staging_buffer, width * height * 4);
        vmaCopyMemoryToAllocation(_context->allocator, _data, _staging_buffer.allocation, 0, width * height * 4);

        stbi_image_free(_data);
    }

    void Texture::_copy_data(const std::uint32_t width, const std::uint32_t height) {
        auto cmd = ScopedCommandBuffer(_context);
        cmd.begin();

        image::transition_layout(
            cmd.get_command_buffer(),
            _info.image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,                                      // srcAccessMask (no need to wait for previous operations)
            VK_ACCESS_TRANSFER_WRITE_BIT,           // dstAccessMask
            VK_PIPELINE_STAGE_HOST_BIT,             // srcStage
            VK_PIPELINE_STAGE_TRANSFER_BIT          // dstStage
        );

        // Copy buffer to image
        VkBufferImageCopy region {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;   // 0 means tightly packed
        region.bufferImageHeight = 0; // 0 means tightly packed
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            cmd.get_command_buffer(),
            _staging_buffer.buffer,
            _info.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        image::transition_layout(
            cmd.get_command_buffer(),
            _info.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT,               // srcAccessMask (no need to wait for previous operations)
            VK_ACCESS_SHADER_READ_BIT,                  // dstAccessMask
            VK_PIPELINE_STAGE_TRANSFER_BIT,             // srcStage
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT       // dstStage
        );
    }

    void Texture::_create_sampler() {
        // Calculate valid filter and mipmap modes
        VkFilter            filter      = VK_FILTER_LINEAR;
        VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        VkSamplerCreateInfo sampler_info {};
        sampler_info.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter    = filter;
        sampler_info.minFilter    = filter;
        sampler_info.mipmapMode   = mipmap_mode;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.mipLodBias   = 0.0f;
        sampler_info.compareOp    = VK_COMPARE_OP_NEVER;
        sampler_info.minLod       = 0.0f;

        validate(
            vkCreateSampler(_context->device, &sampler_info, nullptr, &_info.sampler),
            "Failed to create texture sampler."
        );
    }

    void Texture::_create_view() {
        VkImageViewCreateInfo view_info {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = _info.image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        validate(
            vkCreateImageView(_context->device, &view_info, nullptr, &_info.view),
            "Failed to create texture view."
        );
    }
}
