#include "ignis/core/Core.hh"
#include "ignis/core/Handle.hh"

namespace ignis::render {

enum class ResourceType { renderGraph, frame };

template <ResourceType T>
struct Handle : HandleBase<ResourceType, T> {
    using HandleBase<ResourceType, T>::HandleBase;
};

using RenderGraphHandle = Handle<ResourceType::renderGraph>;
using FrameHandle = Handle<ResourceType::frame>;

}  // namespace ignis::render
