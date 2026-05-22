#pragma once

#include <vector>

#include "VK.hh"
#include "VKShader.hh"
#include "ignis/rhi/BindGroup.hh"
#include "ignis/rhi/Shader.hh"

namespace ignis::rhi {

class VKDevice;

class VKBindGroup : public BindGroupProxy::Impl {
   public:
    explicit VKBindGroup(
        VKDevice& device, const VKShader& shader, VkDescriptorPool pool
    );
    ~VKBindGroup() override;

    VKBindGroup(VKBindGroup&&) noexcept;
    VKBindGroup& operator=(VKBindGroup&&) = delete;

    Opt<Error> set(
        const BindGroupProxy::DescriptorLocation& location,
        TextureHandle texture
    ) override;

    Opt<Error> set(
        const BindGroupProxy::DescriptorLocation& location, BufferHandle buffer
    ) override;

    Opt<Error> set(
        const BindGroupProxy::PushConstantLocation& location, const void* data
    ) override;

    const std::vector<VkDescriptorSet>& descriptorSets() const;
    const std::vector<u8>& pushConstantData() const;
    const std::vector<PushConstantRange>& pushConstantRanges() const;
    VkPipelineLayout pipelineLayout() const;

   private:
    VKDevice& m_device;
    VkDescriptorPool m_pool{VK_NULL_HANDLE};
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
    std::vector<DescriptorBinding> m_bindings;
    std::vector<PushConstantRange> m_pushConstants;
    std::vector<VkDescriptorSet> m_sets;
    std::vector<u8> m_pushConstantData;
};

}  // namespace ignis::rhi
