#include "VKPipeline.hh"

#include "VKDevice.hh"
#include "VKRenderPass.hh"

namespace ignis::rhi {

static u32 formatByteSize(Format fmt) {
    switch (fmt) {
        case Format::r32g32sfloat:
            return 8u;
        case Format::r32g32b32sfloat:
            return 12u;
        case Format::r32g32b32a32sfloat:
            return 16u;
        case Format::r32sfloat:
        case Format::r32sint:
        case Format::r32uint:
            return 4u;
        default:
            log::panic(
                "unsupported vertex attribute format: {}", fmt::underlying(fmt)
            );
    }
}

VKPipeline::VKPipeline(VKDevice& device, const PipelineDescription& desc)
    : m_device(device), m_desc(desc) {
    auto* shader = device.findShader(desc.shader);
    if (not shader) log::panic("VKPipeline: invalid shader handle");

    auto* renderPass = device.findRenderPass(desc.renderPass);
    if (not renderPass) log::panic("VKPipeline: invalid render pass handle");

    auto stages = buildShaderStages(*shader);

    std::vector<VkVertexInputBindingDescription> bindingDescs;
    std::vector<VkVertexInputAttributeDescription> attrDescs;
    VkPipelineVertexInputStateCreateInfo vertexInput{};
    buildVertexInput(
        shader->description(), vertexInput, bindingDescs, attrDescs
    );

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    auto rasterizer = buildRasterizer();
    auto depthStencil = buildDepthStencil();

    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;
    VkPipelineColorBlendStateCreateInfo colorBlend{};
    buildColorBlend(colorBlend, blendAttachments);

    VkPipelineMultisampleStateCreateInfo multisample{};
    multisample.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    constexpr VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<u32>(stages.size());
    pipelineInfo.pStages = stages.data();
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisample;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlend;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = shader->pipelineLayout();
    pipelineInfo.renderPass = renderPass->handle();
    pipelineInfo.subpass = 0;

    VK_ASSERT(vkCreateGraphicsPipelines(
        device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, device.allocator(),
        &m_handle
    ));
}

VKPipeline::~VKPipeline() {
    if (m_handle)
        VK_TRACE(
            vkDestroyPipeline(m_device.device(), m_handle, m_device.allocator())
        );
}

VKPipeline::VKPipeline(VKPipeline&& oth) noexcept
    : m_device(oth.m_device),
      m_desc(std::move(oth.m_desc)),
      m_handle(oth.m_handle) {
    oth.m_handle = VK_NULL_HANDLE;
}

VkPipeline VKPipeline::handle() const { return m_handle; }

const PipelineDescription& VKPipeline::description() const { return m_desc; }

std::vector<VkPipelineShaderStageCreateInfo> VKPipeline::buildShaderStages(
    const VKShader& shader
) const {
    const auto& shaderDesc = shader.description();
    const auto& modules = shader.modules();

    std::vector<VkPipelineShaderStageCreateInfo> stages;
    stages.reserve(shaderDesc.stages.size());
    for (u32 i = 0; i < shaderDesc.stages.size(); ++i) {
        stages.push_back({
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = static_cast<VkShaderStageFlagBits>(
                toVk(shaderDesc.stages[i].stage)
            ),
            .module = modules[i],
            .pName = shaderDesc.stages[i].entrypoint.c_str(),
        });
    }
    return stages;
}

void VKPipeline::buildVertexInput(
    const ShaderDescription& shaderDesc,
    VkPipelineVertexInputStateCreateInfo& vertexInput,
    std::vector<VkVertexInputBindingDescription>& bindingDescs,
    std::vector<VkVertexInputAttributeDescription>& attrDescs
) const {
    if (not shaderDesc.vertexAttributes.empty()) {
        u32 stride = 0;
        for (const auto& attr : shaderDesc.vertexAttributes)
            stride += formatByteSize(attr.format);

        bindingDescs.push_back({
            .binding = 0,
            .stride = stride,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        });

        u32 offset = 0;
        for (const auto& attr : shaderDesc.vertexAttributes) {
            attrDescs.push_back({
                .location = attr.location,
                .binding = 0,
                .format = toVk(attr.format),
                .offset = offset,
            });
            offset += formatByteSize(attr.format);
        }
    }

    vertexInput.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount =
        static_cast<u32>(bindingDescs.size());
    vertexInput.pVertexBindingDescriptions = bindingDescs.data();
    vertexInput.vertexAttributeDescriptionCount =
        static_cast<u32>(attrDescs.size());
    vertexInput.pVertexAttributeDescriptions = attrDescs.data();
}

VkPipelineRasterizationStateCreateInfo VKPipeline::buildRasterizer() const {
    const auto& rast = m_desc.rasterizer;
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = toVk(rast.polygonMode);
    rasterizer.cullMode = toVk(rast.cullMode);
    rasterizer.frontFace = toVk(rast.frontFace);
    rasterizer.lineWidth = rast.lineWidth;
    return rasterizer;
}

VkPipelineDepthStencilStateCreateInfo VKPipeline::buildDepthStencil() const {
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    if (m_desc.depth) {
        depthStencil.depthTestEnable =
            m_desc.depth->testEnable ? VK_TRUE : VK_FALSE;
        depthStencil.depthWriteEnable =
            m_desc.depth->writeEnable ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = toVk(m_desc.depth->compareOp);
    }
    return depthStencil;
}

void VKPipeline::buildColorBlend(
    VkPipelineColorBlendStateCreateInfo& colorBlend,
    std::vector<VkPipelineColorBlendAttachmentState>& attachments
) const {
    attachments.reserve(m_desc.blendAttachments.size());
    for (const auto& b : m_desc.blendAttachments) {
        attachments.push_back({
            .blendEnable = b.blendEnabled ? VK_TRUE : VK_FALSE,
            .srcColorBlendFactor = toVk(b.srcColorBlendFactor),
            .dstColorBlendFactor = toVk(b.dstColorBlendFactor),
            .colorBlendOp = toVk(b.colorBlendOp),
            .srcAlphaBlendFactor = toVk(b.srcAlphaBlendFactor),
            .dstAlphaBlendFactor = toVk(b.dstAlphaBlendFactor),
            .alphaBlendOp = toVk(b.alphaBlendOp),
            .colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        });
    }

    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.logicOpEnable = VK_FALSE;
    colorBlend.attachmentCount = static_cast<u32>(attachments.size());
    colorBlend.pAttachments = attachments.data();
}

}  // namespace ignis::rhi
