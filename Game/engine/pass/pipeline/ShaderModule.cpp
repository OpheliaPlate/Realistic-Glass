#include "ShaderModule.hpp"

#include <fstream>
#include <vector>

#include "Device.hpp"

class ShaderModule::Private
{
public:
    Private(std::shared_ptr<Device> device, const std::string& path)
        : _device(device)
    {
        auto code = readFile(path);
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(*device, &createInfo, nullptr, &_shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
    }
    
    ~Private()
    {
        vkDestroyShaderModule(*_device, _shaderModule, nullptr);
    }

    operator VkShaderModule()
    {
        return _shaderModule;
    }
    
private:
    std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }
    
private:
    VkShaderModule _shaderModule;
    
    std::shared_ptr<Device> _device;
};

ShaderModule::ShaderModule(std::shared_ptr<Device> device, const std::string& path)
    : _delegate(std::make_unique<Private>(device, path))
{
    
}

ShaderModule::~ShaderModule()
{
    
}

ShaderModule::operator VkShaderModule()
{
    return *_delegate;
}
