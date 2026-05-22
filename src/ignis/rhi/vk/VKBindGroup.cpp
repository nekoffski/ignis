#include "VKBindGroup.hh"

#include <algorithm>
#include <cstring>

#include "VKDevice.hh"
#include "VKResourceManager.hh"
#include "VKTexture.hh"

namespace ignis::rhi {

VKBindGroup::VKBindGroup(
    VKDevice& device, const VKShader& shader, VkDescriptorPool pool
)
    : m_device(device),
      m_pool(pool),
      m_pipelineLayout(shader.pipelineLayout()),
      m_bindings(shader.description().bindings),
      m_pushConstants(shader.description().pushConstants) {
    u32 totalPushConstantSize = 0;
    for (const auto& pc : m_pushConstants) {
        totalPushConstantSize =
            std::max(totalPushConstantSize, pc.offset + pc.size);
    }

    m_pushConstantData.resize(totalPushConstantSize, 0);

    const auto& layouts = shader.descriptorSetLayouts();
    if (layouts.empty()) return;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_pool;
    allocInfo.descriptorSetCount = static_cast<u32>(layouts.size());
    allocInfo.pSetLayouts = layouts.data();

    m_sets.resize(layouts.size(), VK_NULL_HANDLE);
    VK_ASSERT(
        vkAllocateDescriptorSets(m_device.device(), &allocInfo, m_sets.data())
    );
}

VKBindGroup::~VKBindGroup() {
    if (m_sets.empty() || m_pool == VK_NULL_HANDLE) return;
    VK_TRACE(vkFreeDescriptorSets(
        m_device.device(), m_pool, static_cast<u32>(m_sets.size()),
        m_sets.data()
    ));
}

VKBindGroup::VKBindGroup(VKBindGroup&& oth) noexcept
    : m_device(oth.m_device),
      m_pool(std::exchange(oth.m_pool, VK_NULL_HANDLE)),
      m_pipelineLayout(std::exchange(oth.m_pipelineLayout, VK_NULL_HANDLE)),
      m_bindings(std::move(oth.m_bindings)),
      m_pushConstants(std::move(oth.m_pushConstants)),
      m_sets(std::move(oth.m_sets)),
      m_pushConstantData(std::move(oth.m_pushConstantData)) {}

Opt<Error> VKBindGroup::set(
    const BindGroupProxy::DescriptorLocation& location, BufferHandle buffer
) {
    auto* vkBuffer = m_device.resources().find(buffer);
    if (not vkBuffer)
        return Error{Error::Code::resourceMissing, "Buffer not found"};

    if (location.set >= m_sets.size()) {
        return Error{
            Error::Code::invalidArgument,
            "Set index {} out of range (bind group has {} sets)", location.set,
            m_sets.size()
        };
    }

    VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    for (const auto& b : m_bindings) {
        if (b.set == location.set && b.binding == location.binding) {
            descriptorType = toVk(b.type);
            break;
        }
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = vkBuffer->handle();
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_sets[location.set];
    write.dstBinding = location.binding;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = descriptorType;
    write.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(m_device.device(), 1, &write, 0, nullptr);
    return Error::empty();
}

Opt<Error> VKBindGroup::set(
    const BindGroupProxy::DescriptorLocation& location, TextureHandle texture
) {
    auto* vkTexture = m_device.resources().find(texture);
    if (not vkTexture) {
        return Error{Error::Code::resourceMissing, "Texture not found"};
    }

    if (location.set >= m_sets.size()) {
        return Error{
            Error::Code::invalidArgument,
            "Set index {} out of range (bind group has {} sets)", location.set,
            m_sets.size()
        };
    }

    VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    for (const auto& b : m_bindings) {
        if (b.set == location.set && b.binding == location.binding) {
            descriptorType = toVk(b.type);
            break;
        }
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = vkTexture->view();
    imageInfo.sampler = vkTexture->sampler();

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_sets[location.set];
    write.dstBinding = location.binding;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = descriptorType;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device.device(), 1, &write, 0, nullptr);
    return Error::empty();
}

Opt<Error> VKBindGroup::set(
    const BindGroupProxy::PushConstantLocation& location, const void* data
) {
    if (location.offset + location.size > m_pushConstantData.size()) {
        return Error{
            Error::Code::invalidArgument,
            "Push constant range [{}, {}) exceeds buffer size {}",
            location.offset, location.offset + location.size,
            m_pushConstantData.size()
        };
    }
    std::memcpy(
        m_pushConstantData.data() + location.offset, data, location.size
    );
    return Error::empty();
}

const std::vector<VkDescriptorSet>& VKBindGroup::descriptorSets() const {
    return m_sets;
}

const std::vector<u8>& VKBindGroup::pushConstantData() const {
    return m_pushConstantData;
}

const std::vector<PushConstantRange>& VKBindGroup::pushConstantRanges() const {
    return m_pushConstants;
}

VkPipelineLayout VKBindGroup::pipelineLayout() const {
    return m_pipelineLayout;
}

}  // namespace ignis::rhi
