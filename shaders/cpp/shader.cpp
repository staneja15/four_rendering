#include "shader.h"
#include "utils/file_system.h"

#include <utility>
#include <utils/error.h>

namespace fr {
    Shader::Shader(std::string shader_name, const VkDevice& device)
        : _shader_name(std::move(shader_name))
        , _device(device)
    { }

    Shader::~Shader() {
        destroy_shaders();
    }

    void Shader::create_shader_program() {
        _read_shader_file();
    }

    std::vector<VkPipelineShaderStageCreateInfo> Shader::get_shader_stages() const {
        return _shader_stages;
    }

    void Shader::destroy_shaders() {
        if (_vertex_shader_module != VK_NULL_HANDLE) {
            vkDestroyShaderModule(_device, _vertex_shader_module, nullptr);
            _vertex_shader_module = VK_NULL_HANDLE;
        }
        if (_fragment_shader_module != VK_NULL_HANDLE) {
            vkDestroyShaderModule(_device, _fragment_shader_module, nullptr);
            _fragment_shader_module = VK_NULL_HANDLE;
        }
    }

    void Shader::_read_shader_file() {
        std::vector<char> vertex_shader_code = file_system::read_binary_file(shader_dir + _shader_name + ".spv");
        std::vector<char> fragment_shader_code = file_system::read_binary_file(shader_dir + _shader_name + ".spv");

        _vertex_shader_module = _create_shader_module(vertex_shader_code);
        _fragment_shader_module = _create_shader_module(fragment_shader_code);

        // Define the vertex shader info
        const auto vertex_info = _create_shader_stage_info(_vertex_shader_module, VK_SHADER_STAGE_VERTEX_BIT, "vertex_main");
        const auto fragment_info = _create_shader_stage_info(_fragment_shader_module, VK_SHADER_STAGE_FRAGMENT_BIT, "fragment_main");

        _shader_stages = {vertex_info, fragment_info};
    }

    VkShaderModule Shader::_create_shader_module(const std::vector<char>& code) {
        VkShaderModuleCreateInfo shader_create_info = {};
        shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_create_info.codeSize = code.size();
        shader_create_info.pCode = reinterpret_cast<const std::uint32_t*>(code.data());

        VkShaderModule shader_module;
        validate(
            vkCreateShaderModule(_device, &shader_create_info, nullptr, &shader_module),
            "Failed to create shader module!"
        );

        return shader_module;
    }

    VkPipelineShaderStageCreateInfo Shader::_create_shader_stage_info(const VkShaderModule& module, const VkShaderStageFlagBits stage, const char* pipeline_name) {
        VkPipelineShaderStageCreateInfo shader_stage_info = {};
        shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_info.stage = stage;
        shader_stage_info.module = module;
        shader_stage_info.pName = pipeline_name;

        return shader_stage_info;
    }
}  // namespace fr
