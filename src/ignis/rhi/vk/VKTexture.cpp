#include "VKTexture.hh"

#include "VKBuffer.hh"
#include "VKCommandBuffer.hh"
#include "VKDevice.hh"

namespace ignis {

VKTexture::VKTexture(VKDevice& device, const DeviceImageDimensions& dim,
                     const DeviceTextureMetadata& metadata,
                     const DeviceSamplerProperties& samplerProps)
    : m_device(device),
      m_ownedBySwapchain(false),
      m_dim(dim),
      m_metadata(metadata),
      m_samplerProps(samplerProps) {
    createImage(dim, metadata);
    bindMemory();
    createView(metadata);
    createSampler(samplerProps);

    log::debug(
        "Created texture with dimensions {}, metadata: {}, sampler "
        "properties: {}",
        toString(dim), toString(metadata), toString(samplerProps));
}

VKTexture::VKTexture(VKDevice& device, VkImage image,
                     const DeviceTextureMetadata& metadata,
                     const DeviceSamplerProperties& samplerProps)
    : m_device(device),
      m_image(image),
      m_ownedBySwapchain(true),
      m_metadata(metadata),
      m_samplerProps(samplerProps) {
    bindMemory();
    createView(metadata);
    createSampler(samplerProps);

    log::debug(
        "Created swapchain-owned texture with metadata: {}, sampler "
        "properties: {}",
        toString(metadata), toString(samplerProps));
}

VKTexture::~VKTexture() {
    auto device = m_device.device();
    auto allocator = m_device.allocator();

    if (m_sampler != VK_NULL_HANDLE)
        VK_TRACE(vkDestroySampler(device, m_sampler, allocator));

    if (m_view != VK_NULL_HANDLE) vkDestroyImageView(device, m_view, allocator);

    if (not m_ownedBySwapchain) {
        if (m_memory != VK_NULL_HANDLE)
            VK_TRACE(vkFreeMemory(device, m_memory, allocator));

        if (m_image != VK_NULL_HANDLE)
            VK_TRACE(vkDestroyImage(device, m_image, allocator));
    }
}

void VKTexture::bindMemory() {
    VkMemoryRequirements memoryRequirements;
    VK_TRACE(vkGetImageMemoryRequirements(m_device.device(), m_image,
                                          &memoryRequirements));

    auto memoryType = m_device.findMemoryIndex(
        memoryRequirements.memoryTypeBits, DeviceMemoryProperty::deviceLocal);

    if (not memoryType)
        log::error("Required memory type not found. VKImage not valid.");

    VkMemoryAllocateInfo memoryAllocateInfo{};

    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryType.value_or(-1);

    VK_ASSERT(vkAllocateMemory(m_device.device(), &memoryAllocateInfo,
                               m_device.allocator(), &m_memory));
    VK_ASSERT(vkBindImageMemory(m_device.device(), m_image, m_memory, 0));
}

void VKTexture::createImage(const DeviceImageDimensions& dim,
                            const DeviceTextureMetadata& metadata) {
    log::expect(
        m_device.supportsFormat(metadata.format, metadata.tiling,
                                metadata.usage),
        "Device does not support the requested texture "
        "format/tiling/usage combination - format: {}, tiling: {}, usage: {}",
        toString(metadata.format), toString(metadata.tiling),
        toString(metadata.usage));

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = dim.width;
    imageCreateInfo.extent.height = dim.height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = metadata.mipLevels;
    imageCreateInfo.arrayLayers = metadata.arrayLayers;
    imageCreateInfo.format = toVk(metadata.format);
    imageCreateInfo.tiling = toVk(metadata.tiling);
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = toVk(metadata.usage);
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (metadata.type == DeviceTextureType::cubemap)
        imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VK_ASSERT(vkCreateImage(m_device.device(), &imageCreateInfo,
                            m_device.allocator(), &m_image));
}

void VKTexture::createView(const DeviceTextureMetadata& metadata) {
    VkImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    viewCreateInfo.viewType = metadata.type == DeviceTextureType::cubemap
                                  ? VK_IMAGE_VIEW_TYPE_CUBE
                                  : VK_IMAGE_VIEW_TYPE_2D;

    viewCreateInfo.format = toVk(metadata.format);
    viewCreateInfo.subresourceRange.aspectMask = toVk(metadata.aspect);
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.layerCount = metadata.arrayLayers;
    viewCreateInfo.image = m_image;

    VK_ASSERT(vkCreateImageView(m_device.device(), &viewCreateInfo,
                                m_device.allocator(), &m_view));
}

void VKTexture::createSampler(const DeviceSamplerProperties& samplerProps) {
    static std::unordered_map<DeviceTextureRepeat, VkSamplerAddressMode>
        vkRepeat{
            {DeviceTextureRepeat::repeat, VK_SAMPLER_ADDRESS_MODE_REPEAT},
            {DeviceTextureRepeat::mirroredRepeat,
             VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT},
            {DeviceTextureRepeat::clampToEdge,
             VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE},
            {DeviceTextureRepeat::clampToBorder,
             VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER},
        };
    static std::unordered_map<DeviceTextureFilter, VkFilter> vkFilter{
        {DeviceTextureFilter::nearest, VK_FILTER_NEAREST},
        {DeviceTextureFilter::linear, VK_FILTER_LINEAR},
    };

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = vkFilter[samplerProps.magnifyFilter];
    samplerInfo.minFilter = vkFilter[samplerProps.minifyFilter];
    samplerInfo.addressModeU = vkRepeat[samplerProps.uRepeat];
    samplerInfo.addressModeV = vkRepeat[samplerProps.vRepeat];
    samplerInfo.addressModeW = vkRepeat[samplerProps.wRepeat];
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VK_ASSERT(vkCreateSampler(m_device.device(), &samplerInfo,
                              m_device.allocator(), &m_sampler));
}

VkImage VKTexture::image() const { return m_image; }

VkSampler VKTexture::sampler() const { return m_sampler; }

VkImageView VKTexture::view() const { return m_view; }

bool VKTexture::ownedBySwapchain() const { return m_ownedBySwapchain; }

VkImageLayout& VKTexture::layout() { return m_layout; }

void VKTexture::copyFrom(VKBuffer& buffer, VkCommandBuffer cmdBuffer) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {m_dim.width, m_dim.height, 1};

    vkCmdCopyBufferToImage(cmdBuffer, buffer.handle(), m_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VKTexture::copyTo(VKBuffer& buffer, VkCommandBuffer cmdBuffer) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {m_dim.width, m_dim.height, 1};

    vkCmdCopyImageToBuffer(cmdBuffer, m_image,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           buffer.handle(), 1, &region);
}

namespace {

struct LayoutSyncInfo {
    VkPipelineStageFlags stage;
    VkAccessFlags access;
};

LayoutSyncInfo syncInfoForLayout(VkImageLayout layout) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0};
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return {VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_ACCESS_TRANSFER_WRITE_BIT};
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return {VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_ACCESS_TRANSFER_READ_BIT};
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return {VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    VK_ACCESS_SHADER_READ_BIT};
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return {VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0};
        default:
            return {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT};
    }
}

}  // namespace

VKTexture::Transition VKTexture::transitionLayout(
    VkCommandBuffer cmdBuffer, const Transition& transition) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = m_layout;
    barrier.newLayout = transition.newLayout;
    barrier.srcQueueFamilyIndex = m_device.queueIndex(transition.srcQueue);
    barrier.dstQueueFamilyIndex = m_device.queueIndex(transition.dstQueue);
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = transition.srcAccessMask;
    barrier.dstAccessMask = transition.dstAccessMask;

    vkCmdPipelineBarrier(cmdBuffer, transition.srcStageMask,
                         transition.dstStageMask, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);

    Transition reverse{
        .newLayout = m_layout,
        .srcStageMask = transition.dstStageMask,
        .dstStageMask = transition.srcStageMask,
        .srcAccessMask = transition.dstAccessMask,
        .dstAccessMask = transition.srcAccessMask,
        .srcQueue = transition.dstQueue,
        .dstQueue = transition.srcQueue,
    };

    m_layout = transition.newLayout;
    return reverse;
}

VKTexture::Transition VKTexture::transitionLayout(VkCommandBuffer cmdBuffer,
                                                  VkImageLayout newLayout) {
    auto src = syncInfoForLayout(m_layout);
    auto dst = syncInfoForLayout(newLayout);

    Transition transition{
        .newLayout = newLayout,
        .srcStageMask = src.stage,
        .dstStageMask = dst.stage,
        .srcAccessMask = src.access,
        .dstAccessMask = dst.access,
    };

    return transitionLayout(cmdBuffer, transition);
}

VKTexture::VKTexture(VKTexture&& other) noexcept
    : m_ownedBySwapchain(other.m_ownedBySwapchain),
      m_device(other.m_device),
      m_image(other.m_image),
      m_sampler(other.m_sampler),
      m_view(other.m_view),
      m_memory(other.m_memory),
      m_layout(other.m_layout),
      m_dim(other.m_dim),
      m_metadata(other.m_metadata),
      m_samplerProps(other.m_samplerProps) {
    other.m_image = VK_NULL_HANDLE;
    other.m_sampler = VK_NULL_HANDLE;
    other.m_view = VK_NULL_HANDLE;
    other.m_memory = VK_NULL_HANDLE;
    other.m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

}  // namespace ignis
