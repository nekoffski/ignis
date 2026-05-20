#include "VKBindGroup.hh"

#include "VKDevice.hh"

namespace ignis::rhi {

VKBindGroup::VKBindGroup(VKDevice& device, VKShader& shader)
    : m_device(device), m_shader(shader) {}

VKBindGroup::~VKBindGroup() {}

VKBindGroup::VKBindGroup(VKBindGroup&& oth)
    : m_device(oth.m_device),
      m_shader(oth.m_shader),
      m_sets(std::move(oth.m_sets)),
      m_pushConstantData(std::move(oth.m_pushConstantData)) {}

const std::vector<VkDescriptorSet>& VKBindGroup::descriptorSets() const {
    return m_sets;
}

const std::vector<u8>& VKBindGroup::pushConstantData() const {
    return m_pushConstantData;
}

}  // namespace ignis::rhi
