#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace fr {
    class Shader {
    public:
        std::string shader_dir = "/opt/four_map_engine/four_rendering/shaders/spirv/";

        explicit Shader(std::string shader_name, const VkDevice& device);

        ~Shader();

        void create_shader_program();

        [[nodiscard]] std::vector<VkPipelineShaderStageCreateInfo> get_shader_stages() const;

        void destroy_shaders();

    private:
        std::string _shader_name;
        VkDevice _device;
        VkShaderModule _vertex_shader_module {};
        VkShaderModule _fragment_shader_module {};
        std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;

        void _read_shader_file();

        VkShaderModule _create_shader_module(const std::vector<char>& code);

        static VkPipelineShaderStageCreateInfo _create_shader_stage_info(const VkShaderModule& module, VkShaderStageFlagBits stage, const char* pipeline_name);
    };
}  // namespace fr