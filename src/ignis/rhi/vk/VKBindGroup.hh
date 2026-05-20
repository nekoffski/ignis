#pragma once

#include <vector>

#include "VK.hh"
#include "VKShader.hh"
#include "ignis/rhi/BindGroup.hh"

namespace ignis::rhi {

class VKDevice;

class VKBindGroup : public BindGroupProxy::Impl {
   public:
    explicit VKBindGroup(VKDevice& device, VKShader& shader);
    ~VKBindGroup() override;

    VKBindGroup(VKBindGroup&&);
    VKBindGroup& operator=(VKBindGroup&&) = delete;

    const std::vector<VkDescriptorSet>& descriptorSets() const;
    const std::vector<u8>& pushConstantData() const;

   private:
    VKDevice& m_device;
    VKShader& m_shader;

    std::vector<VkDescriptorSet> m_sets;
    std::vector<u8> m_pushConstantData;
};

}  // namespace ignis::rhi
