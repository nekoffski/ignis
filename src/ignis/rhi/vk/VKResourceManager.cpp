#include "VKResourceManager.hh"

#include "VKDevice.hh"

namespace ignis::rhi {

VKResourceManager::VKResourceManager(VKDevice& device, const Config& config)
    : m_device(device),
      m_bufferPool(config.renderer().maxBuffers),
      m_texturePool(config.renderer().maxTextures),
      m_renderPassPool(config.renderer().maxRenderPasses),
      m_shaderPool(config.renderer().maxShaders),
      m_pipelinePool(config.renderer().maxPipelines) {}

Result<BufferHandle> VKResourceManager::create(const BufferDescription& desc) {
    auto id = m_bufferPool.create([&](u32 slot) {
        return ResourceWrapper{
            VKBuffer{
                m_device,
                desc,
            },
            BufferHandle{slot},
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

void VKResourceManager::destroy(BufferHandle handle) {
    m_bufferPool.destroy(handle.id);
}

Result<TextureHandle> VKResourceManager::create(
    const TextureDescription& definition
) {
    auto id = m_texturePool.create([&](u32 slot) {
        return ResourceWrapper{
            VKTexture{
                m_device,
                definition.image,
                definition.metadata,
                definition.sampler,
            },
            TextureHandle{slot},
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
    auto id = m_renderPassPool.create([&](u32 slot) {
        return ResourceWrapper{
            VKRenderPass{
                m_device,
                desc,
            },
            RenderPassHandle{slot},
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

void VKResourceManager::destroy(RenderPassHandle handle) {
    m_renderPassPool.destroy(handle.id);
}

Result<ShaderHandle> VKResourceManager::create(
    const ShaderDescription& shaderDescription
) {
    auto id = m_shaderPool.create([&](u32 slot) {
        return ResourceWrapper{
            VKShader{m_device, shaderDescription},
            ShaderHandle{slot},
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

void VKResourceManager::destroy(ShaderHandle handle) {
    m_shaderPool.destroy(handle.id);
}

Result<PipelineHandle> VKResourceManager::create(
    const PipelineDescription& pipelineDescription
) {
    auto id = m_pipelinePool.create([&](u32 slot) {
        return ResourceWrapper{
            VKPipeline{m_device, pipelineDescription},
            PipelineHandle{slot},
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

void VKResourceManager::destroy(PipelineHandle handle) {
    m_pipelinePool.destroy(handle.id);
}

VKPipeline* VKResourceManager::find(PipelineHandle handle) {
    if (auto wrapper = m_pipelinePool.get(handle.id); wrapper)
        return &wrapper->resource;
    log::warn(
        "Failed to get pipeline: invalid pipeline handle: {}",
        static_cast<u32>(handle.id)
    );
    return nullptr;
}

void VKResourceManager::destroy(TextureHandle handle) {
    m_texturePool.destroy(handle.id);
}

VKTexture* VKResourceManager::find(TextureHandle handle) {
    if (auto textureWrapper = m_texturePool.get(handle.id); textureWrapper)
        return &textureWrapper->resource;
    log::warn(
        "Failed to get texture proxy: invalid texture handle: {}",
        static_cast<u32>(handle.id)
    );
    return nullptr;
}

VKBuffer* VKResourceManager::find(BufferHandle handle) {
    if (auto bufferWrapper = m_bufferPool.get(handle.id); bufferWrapper)
        return &bufferWrapper->resource;
    log::warn(
        "Failed to get buffer proxy: invalid buffer handle: {}",
        static_cast<u32>(handle.id)
    );
    return nullptr;
}

VKRenderPass* VKResourceManager::find(RenderPassHandle handle) {
    if (auto renderPassWrapper = m_renderPassPool.get(handle.id);
        renderPassWrapper)
        return &renderPassWrapper->resource;
    log::warn(
        "Failed to get render pass proxy: invalid render pass handle: {}",
        static_cast<u32>(handle.id)
    );
    return nullptr;
}

VKShader* VKResourceManager::find(ShaderHandle handle) {
    if (auto shaderWrapper = m_shaderPool.get(handle.id); shaderWrapper)
        return &shaderWrapper->resource;
    log::warn(
        "Failed to get shader: invalid shader handle: {}",
        static_cast<u32>(handle.id)
    );
    return nullptr;
}

BufferProxy::Impl* VKResourceManager::proxy(BufferHandle handle) {
    if (auto bufferWrapper = m_bufferPool.get(handle.id); bufferWrapper)
        return &bufferWrapper->resource;
    log::error("Failed to get buffer proxy: invalid buffer handle");
    return nullptr;
}

}  // namespace ignis::rhi
