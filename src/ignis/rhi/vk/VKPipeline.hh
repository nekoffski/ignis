#pragma once

#include "VK.hh"
#include "VKShader.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/Pipeline.hh"

namespace ignis::rhi {

class VKDevice;

class VKPipeline : public NonCopyable {
   public:
    explicit VKPipeline(VKDevice& device, const PipelineDescription& desc);
    ~VKPipeline();

    VKPipeline(VKPipeline&&) noexcept;
    VKPipeline& operator=(VKPipeline&&) noexcept = delete;

    VkPipeline handle() const;
    const PipelineDescription& description() const;

   private:
    std::vector<VkPipelineShaderStageCreateInfo> buildShaderStages(
        const VKShader& shader
    ) const;

    void buildVertexInput(
        const ShaderDescription& shaderDesc,
        VkPipelineVertexInputStateCreateInfo& vertexInput,
        std::vector<VkVertexInputBindingDescription>& bindingDescs,
        std::vector<VkVertexInputAttributeDescription>& attrDescs
    ) const;

    VkPipelineRasterizationStateCreateInfo buildRasterizer() const;
    VkPipelineDepthStencilStateCreateInfo buildDepthStencil() const;

    void buildColorBlend(
        VkPipelineColorBlendStateCreateInfo& colorBlend,
        std::vector<VkPipelineColorBlendAttachmentState>& attachments
    ) const;

    VKDevice& m_device;
    PipelineDescription m_desc;
    VkPipeline m_handle{VK_NULL_HANDLE};
};

}  // namespace ignis::rhi
