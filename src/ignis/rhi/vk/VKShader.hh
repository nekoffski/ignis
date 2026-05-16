#pragma once

#include <vector>

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/Shader.hh"

namespace ignis::rhi {

class VKDevice;

class VKShader : public NonCopyable {
   public:
    explicit VKShader(VKDevice& device, const ShaderDescription& desc);
    ~VKShader();

    VKShader(VKShader&&) noexcept;
    VKShader& operator=(VKShader&&) noexcept = delete;

    VkPipelineLayout pipelineLayout() const;
    const std::vector<VkShaderModule>& modules() const;
    const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts() const;
    const ShaderDescription& description() const;

   private:
    void createShaderModules(const ShaderDescription& desc);
    void createDescriptorSetLayouts(const ShaderDescription& desc);
    void createPipelineLayout(const ShaderDescription& desc);

    VKDevice& m_device;
    ShaderDescription m_desc;
    std::vector<VkShaderModule> m_modules;
    std::vector<VkDescriptorSetLayout> m_setLayouts;
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
};

}  // namespace ignis::rhi
