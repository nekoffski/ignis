#pragma once

#include "BindGroup.hh"
#include "Buffer.hh"
#include "Pipeline.hh"
#include "RenderPass.hh"
#include "Shader.hh"
#include "Texture.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis::rhi {

class ResourceManager : public virtual NonCopyable, public virtual NonMovable {
    friend class BufferProxy;
    friend class BindGroupProxy;

   public:
    virtual ~ResourceManager() = default;

    virtual Result<BufferHandle> create(
        const BufferDescription& bufferDescription
    ) = 0;
    virtual Opt<Error> destroy(BufferHandle handle) = 0;

    virtual Result<TextureHandle> create(
        const TextureDescription& textureDescription
    ) = 0;
    virtual Opt<Error> destroy(TextureHandle handle) = 0;

    virtual Result<RenderPassHandle> create(
        const RenderPassDescription& renderPassDescription
    ) = 0;
    virtual Opt<Error> destroy(RenderPassHandle handle) = 0;

    virtual Result<ShaderHandle> create(
        const ShaderDescription& shaderDescription
    ) = 0;
    virtual Opt<Error> destroy(ShaderHandle handle) = 0;

    virtual Result<PipelineHandle> create(
        const PipelineDescription& pipelineDescription
    ) = 0;
    virtual Opt<Error> destroy(PipelineHandle handle) = 0;

    virtual Result<BindGroupHandle> create(
        const BindGroupDescription& bindGroupDescription
    ) = 0;
    virtual Opt<Error> destroy(BindGroupHandle handle) = 0;

   private:
    virtual BufferProxy::Impl* proxy(BufferHandle handle) = 0;
    virtual BindGroupProxy::Impl* proxy(BindGroupHandle handle) = 0;
};

}  // namespace ignis::rhi
