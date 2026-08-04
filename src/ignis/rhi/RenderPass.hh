#pragma once

#include <vector>

#include "ResourceHandle.hh"
#include "Texture.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis::rhi {

struct Attachment {
    bool clear{false};
    Format format{Format::undefined};
    Layout initialLayout{Layout::undefined};
    Layout finalLayout{Layout::undefined};
};

struct RenderPassDescription {
    std::vector<Attachment> colorAttachments;
    std::optional<Attachment> depthAttachment;
};

}  // namespace ignis::rhi
