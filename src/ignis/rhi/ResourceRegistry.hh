#pragma once

#include <memory>

#include "Buffer.hh"
#include "Device.hh"
#include "Texture.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"

namespace ignis {

class ResourceRegistry : public NonCopyable, public NonMovable {
   public:
    static std::unique_ptr<ResourceRegistry> create(const Config& config,
                                                    Device& device);

    virtual ~ResourceRegistry() = default;

    virtual BufferHandle createBuffer(const BufferDescription& desc) = 0;
    virtual void destroyBuffer(BufferHandle handle) = 0;

    virtual TextureHandle createTexture(const TextureDescription& desc) = 0;
    virtual void destroyTexture(TextureHandle handle) = 0;
};

}  // namespace ignis
