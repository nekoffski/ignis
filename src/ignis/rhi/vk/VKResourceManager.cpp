#include "VKResourceManager.hh"

#include <string_view>

#include "VKDevice.hh"

namespace ignis::rhi {

namespace {

template <typename Pool, typename Handle>
Opt<Error> destroyResource(
    Pool& pool, Handle handle, std::string_view resourceType
) {
    if (not pool.destroy(handle)) {
        return Error{
            Error::Code::resourceMissing,
            "Failed to destroy {}: invalid handle {}:{}", resourceType,
            handle.id, handle.generation
        };
    }
    return Error::empty();
}

}  // namespace

VKResourceManager::VKResourceManager(VKDevice& device, const Config& config)
    : m_device(device),
      m_maxBindGroups(static_cast<u32>(config.renderer().maxBindGroups)),
      m_bufferPool(config.renderer().maxBuffers),
      m_texturePool(config.renderer().maxTextures),
      m_renderPassPool(config.renderer().maxRenderPasses),
      m_shaderPool(config.renderer().maxShaders),
      m_pipelinePool(config.renderer().maxPipelines),
      m_bindGroupPool(config.renderer().maxBindGroups) {}

void VKResourceManager::ensureDescriptorPool() {
    if (not m_descriptorPool.empty()) {
        return;
    }

    auto maxGroups = m_maxBindGroups;

    std::array<VkDescriptorPoolSize, 5> sizes{{
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxGroups * 8},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, maxGroups * 8},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, maxGroups * 8},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, maxGroups * 4},
        {VK_DESCRIPTOR_TYPE_SAMPLER, maxGroups * 4},
    }};

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = maxGroups * 8;
    poolInfo.poolSizeCount = static_cast<u32>(sizes.size());
    poolInfo.pPoolSizes = sizes.data();

    VkDescriptorPool pool = VK_NULL_HANDLE;
    VK_ASSERT(vkCreateDescriptorPool(
        m_device.device(), &poolInfo, m_device.allocator(), &pool
    ));

    m_descriptorPool =
        Scoped<VkDescriptorPool>(pool, [this](VkDescriptorPool& p) {
            if (p != VK_NULL_HANDLE) {
                VK_TRACE(vkDestroyDescriptorPool(
                    m_device.device(), p, m_device.allocator()
                ));
            }
        });
}

Result<BufferHandle> VKResourceManager::create(const BufferDescription& desc) {
    auto id = m_bufferPool.create([&](HandleKey key) {
        return ResourceWrapper{
            VKBuffer{
                m_device,
                desc,
            },
            BufferHandle{key.id, key.generation},
        };
    });

    if (not id) {
        return Error::unexpected(
            Error::Code::poolFull,
            "Failed to create buffer: buffer pool is full"
        );
    }
    return m_bufferPool.get(*id)->handle;
}

Opt<Error> VKResourceManager::destroy(BufferHandle handle) {
    return destroyResource(m_bufferPool, handle, "buffer");
}

Result<TextureHandle> VKResourceManager::create(
    const TextureDescription& definition
) {
    auto id = m_texturePool.create([&](HandleKey key) {
        return ResourceWrapper{
            VKTexture{
                m_device,
                definition.image,
                definition.metadata,
                definition.sampler,
            },
            TextureHandle{key.id, key.generation},
        };
    });
    if (not id) {
        return Error::unexpected(
            Error::Code::poolFull,
            "Failed to create texture: texture pool is full"
        );
    }
    return m_texturePool.get(*id)->handle;
}

Result<RenderPassHandle> VKResourceManager::create(
    const RenderPassDescription& desc
) {
    auto id = m_renderPassPool.create([&](HandleKey key) {
        return ResourceWrapper{
            VKRenderPass{
                m_device,
                desc,
            },
            RenderPassHandle{key.id, key.generation},
        };
    });
    if (not id) {
        return Error::unexpected(
            Error::Code::poolFull,
            "Failed to create render pass: render pass pool is full"
        );
    }
    return m_renderPassPool.get(*id)->handle;
}

Opt<Error> VKResourceManager::destroy(RenderPassHandle handle) {
    return destroyResource(m_renderPassPool, handle, "render pass");
}

Result<ShaderHandle> VKResourceManager::create(
    const ShaderDescription& shaderDescription
) {
    auto id = m_shaderPool.create([&](HandleKey key) {
        return ResourceWrapper{
            VKShader{m_device, shaderDescription},
            ShaderHandle{key.id, key.generation},
        };
    });
    if (not id) {
        return Error::unexpected(
            Error::Code::poolFull,
            "Failed to create shader: shader pool is full"
        );
    }
    return m_shaderPool.get(*id)->handle;
}

Opt<Error> VKResourceManager::destroy(ShaderHandle handle) {
    return destroyResource(m_shaderPool, handle, "shader");
}

Result<PipelineHandle> VKResourceManager::create(
    const PipelineDescription& pipelineDescription
) {
    auto id = m_pipelinePool.create([&](HandleKey key) {
        return ResourceWrapper{
            VKPipeline{m_device, pipelineDescription},
            PipelineHandle{key.id, key.generation},
        };
    });
    if (not id) {
        return Error::unexpected(
            Error::Code::poolFull,
            "Failed to create pipeline: pipeline pool is full"
        );
    }
    return m_pipelinePool.get(*id)->handle;
}

Opt<Error> VKResourceManager::destroy(PipelineHandle handle) {
    return destroyResource(m_pipelinePool, handle, "pipeline");
}

VKPipeline* VKResourceManager::find(PipelineHandle handle) {
    if (auto wrapper = m_pipelinePool.get(handle); wrapper) {
        return &wrapper->resource;
    }
    log::warn(
        "Failed to get pipeline: invalid pipeline handle: {}",
        static_cast<u32>(handle.id)
    );
    return nullptr;
}

Opt<Error> VKResourceManager::destroy(TextureHandle handle) {
    return destroyResource(m_texturePool, handle, "texture");
}

Result<BindGroupHandle> VKResourceManager::create(
    const BindGroupDescription& bindGroupDescription
) {
    auto* shader = find(bindGroupDescription.shader);
    if (not shader) {
        return Error::unexpected(
            Error::Code::invalidArgument,
            "Failed to create bind group: invalid shader handle: {}",
            static_cast<u32>(bindGroupDescription.shader.id)
        );
    }

    ensureDescriptorPool();

    auto id = m_bindGroupPool.create([&](HandleKey key) {
        return ResourceWrapper{
            VKBindGroup{m_device, *shader, *m_descriptorPool},
            BindGroupHandle{key.id, key.generation},
        };
    });
    if (not id) {
        return Error::unexpected(
            Error::Code::poolFull,
            "Failed to create bind group: bind group pool is full"
        );
    }
    return m_bindGroupPool.get(*id)->handle;
}

Opt<Error> VKResourceManager::destroy(BindGroupHandle handle) {
    return destroyResource(m_bindGroupPool, handle, "bind group");
}

VKTexture* VKResourceManager::find(TextureHandle handle) {
    if (auto textureWrapper = m_texturePool.get(handle); textureWrapper) {
        return &textureWrapper->resource;
    }
    log::warn(
        "Failed to get texture proxy: invalid texture handle: {}",
        static_cast<u32>(handle.id)
    );
    return nullptr;
}

VKBuffer* VKResourceManager::find(BufferHandle handle) {
    if (auto bufferWrapper = m_bufferPool.get(handle); bufferWrapper) {
        return &bufferWrapper->resource;
    }
    log::warn(
        "Failed to get buffer proxy: invalid buffer handle: {}",
        static_cast<u32>(handle.id)
    );
    return nullptr;
}

VKRenderPass* VKResourceManager::find(RenderPassHandle handle) {
    if (auto renderPassWrapper = m_renderPassPool.get(handle);
        renderPassWrapper) {
        return &renderPassWrapper->resource;
    }
    log::warn(
        "Failed to get render pass proxy: invalid render pass handle: {}",
        static_cast<u32>(handle.id)
    );
    return nullptr;
}

VKShader* VKResourceManager::find(ShaderHandle handle) {
    if (auto shaderWrapper = m_shaderPool.get(handle); shaderWrapper) {
        return &shaderWrapper->resource;
    }
    log::warn(
        "Failed to get shader: invalid shader handle: {}",
        static_cast<u32>(handle.id)
    );
    return nullptr;
}

VKBindGroup* VKResourceManager::find(BindGroupHandle handle) {
    if (auto bindGroupWrapper = m_bindGroupPool.get(handle); bindGroupWrapper) {
        return &bindGroupWrapper->resource;
    }
    log::warn(
        "Failed to get bind group: invalid bind group handle: {}",
        static_cast<u32>(handle.id)
    );
    return nullptr;
}

BufferProxy::Impl* VKResourceManager::proxy(BufferHandle handle) {
    return find(handle);
}

BindGroupProxy::Impl* VKResourceManager::proxy(BindGroupHandle handle) {
    return find(handle);
}

}  // namespace ignis::rhi
