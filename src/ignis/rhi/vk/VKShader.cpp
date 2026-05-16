#include "VKShader.hh"

#include <algorithm>
#include <unordered_map>

#include "VKDevice.hh"

namespace ignis::rhi {

VKShader::VKShader(VKDevice& device, const ShaderDescription& desc)
    : m_device(device), m_desc(desc) {
    createShaderModules(desc);
    createDescriptorSetLayouts(desc);
    createPipelineLayout(desc);
}

VKShader::~VKShader() {
    auto vkDevice = m_device.device();
    auto allocator = m_device.allocator();

    if (m_pipelineLayout)
        VK_TRACE(vkDestroyPipelineLayout(vkDevice, m_pipelineLayout, allocator)
        );

    for (auto layout : m_setLayouts)
        VK_TRACE(vkDestroyDescriptorSetLayout(vkDevice, layout, allocator));

    for (auto module : m_modules)
        VK_TRACE(vkDestroyShaderModule(vkDevice, module, allocator));
}

VKShader::VKShader(VKShader&& oth) noexcept
    : m_device(oth.m_device),
      m_desc(std::move(oth.m_desc)),
      m_modules(std::move(oth.m_modules)),
      m_setLayouts(std::move(oth.m_setLayouts)),
      m_pipelineLayout(oth.m_pipelineLayout) {
    oth.m_pipelineLayout = VK_NULL_HANDLE;
}

VkPipelineLayout VKShader::pipelineLayout() const { return m_pipelineLayout; }

const std::vector<VkShaderModule>& VKShader::modules() const {
    return m_modules;
}

const std::vector<VkDescriptorSetLayout>& VKShader::descriptorSetLayouts(
) const {
    return m_setLayouts;
}

const ShaderDescription& VKShader::description() const { return m_desc; }

void VKShader::createShaderModules(const ShaderDescription& desc) {
    m_modules.reserve(desc.stages.size());
    for (const auto& stage : desc.stages) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = stage.spirv.size() * sizeof(u32);
        createInfo.pCode = stage.spirv.data();

        VkShaderModule module{};
        VK_ASSERT(vkCreateShaderModule(
            m_device.device(), &createInfo, m_device.allocator(), &module
        ));
        m_modules.push_back(module);
    }
}

void VKShader::createDescriptorSetLayouts(const ShaderDescription& desc) {
    std::unordered_map<u32, std::vector<VkDescriptorSetLayoutBinding>> sets;

    for (const auto& b : desc.bindings) {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = b.binding;
        layoutBinding.descriptorType = toVk(b.type);
        layoutBinding.descriptorCount = b.count;
        layoutBinding.stageFlags = toVk(b.stageFlags);
        layoutBinding.pImmutableSamplers = nullptr;
        sets[b.set].push_back(layoutBinding);
    }

    u32 maxSet = 0;

    if (not sets.empty()) {
        maxSet =
            std::max_element(
                sets.begin(), sets.end(),
                [](const auto& a, const auto& b) { return a.first < b.first; }
            )->first;
    }

    m_setLayouts.resize(maxSet + (sets.empty() ? 0 : 1), VK_NULL_HANDLE);

    for (auto& [setIndex, bindings] : sets) {
        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = static_cast<u32>(bindings.size());
        createInfo.pBindings = bindings.data();

        VK_ASSERT(vkCreateDescriptorSetLayout(
            m_device.device(), &createInfo, m_device.allocator(),
            &m_setLayouts[setIndex]
        ));
    }
}

void VKShader::createPipelineLayout(const ShaderDescription& desc) {
    std::vector<VkPushConstantRange> pushRanges;
    pushRanges.reserve(desc.pushConstants.size());
    for (const auto& pc : desc.pushConstants) {
        pushRanges.push_back({
            .stageFlags = toVk(pc.stageFlags),
            .offset = pc.offset,
            .size = pc.size,
        });
    }

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = static_cast<u32>(m_setLayouts.size());
    createInfo.pSetLayouts = m_setLayouts.data();
    createInfo.pushConstantRangeCount = static_cast<u32>(pushRanges.size());
    createInfo.pPushConstantRanges = pushRanges.data();

    VK_ASSERT(vkCreatePipelineLayout(
        m_device.device(), &createInfo, m_device.allocator(), &m_pipelineLayout
    ));
}

}  // namespace ignis::rhi
