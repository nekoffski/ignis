#include "VKTexture.hh"

#include "VKDevice.hh"

namespace ignis {

VKTexture::VKTexture(VKDevice& device, const DeviceImageProperties& imgProps,
                     const DeviceViewProperties& viewProps,
                     const DeviceSamplerProperties& samplerProps)
    : m_device(device), m_ownedBySwapchain(false) {
    createImage(imgProps, viewProps);
    createView(viewProps);
    createSampler(samplerProps);
}

VKTexture::VKTexture(VKDevice& device, VkImage image,
                     const DeviceViewProperties& viewProps,
                     const DeviceSamplerProperties& samplerProps)
    : m_device(device), m_image(image), m_ownedBySwapchain(true) {
    bindMemory();
    createView(viewProps);
    createSampler(samplerProps);
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

    // auto memoryType = m_device.findMemoryIndex(
    //     memoryRequirements.memoryTypeBits,
    //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // if (not memoryType)
    //     log::error("Required memory type not found. VKImage not valid.");

    // VkMemoryAllocateInfo memoryAllocateInfo;
    // clearMemory(&memoryAllocateInfo);
    // memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    // memoryAllocateInfo.allocationSize = memoryRequirements.size;
    // memoryAllocateInfo.memoryTypeIndex = memoryType.value_or(-1);

    // log::vkExpect(vkAllocateMemory(m_device.logical.handle,
    // &memoryAllocateInfo,
    //                                m_device.allocator, &m_memory));
    // log::trace("vkAllocateMemory: {}", static_cast<void*>(m_memory));
    // log::vkExpect(
    //     vkBindImageMemory(m_device.logical.handle, m_image, m_memory, 0));
}

void VKTexture::createImage(const DeviceImageProperties& imgProps,
                            const DeviceViewProperties& viewProps) {
    bool isCubemap = viewProps.type == DeviceTextureType::cubemap;

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = imgProps.width;
    imageCreateInfo.extent.height = imgProps.height;
    imageCreateInfo.extent.depth = 1;
    // imageCreateInfo.mipLevels = imgProps.mipLevels;
    imageCreateInfo.arrayLayers = isCubemap ? 6 : 1;
    // imageCreateInfo.format = toVk(imgProps.format, imgProps.channels);
    // imageCreateInfo.tiling = toVk(imgProps.tiling);
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // imageCreateInfo.usage = toVk(imgProps.usage);
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (isCubemap) imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VK_ASSERT(vkCreateImage(m_device.device(), &imageCreateInfo,
                            m_device.allocator(), &m_image));
}

void VKTexture::createView(const DeviceViewProperties& viewProps) {
    VkImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    bool isCubemap = viewProps.type == DeviceTextureType::cubemap;

    viewCreateInfo.viewType =
        isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    // viewCreateInfo.format = toVk(imageProperties.format,
    // imageProperties.channels); viewCreateInfo.subresourceRange.aspectMask =
    // toVk(imageProperties.aspect);
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.layerCount = isCubemap ? 6 : 1;
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

}  // namespace ignis