#pragma once

#include "VKCommandDispatcher.hh"

namespace ignis::rhi {

void preprocessCommand(
    VKCommandManifest& manifest, const CmdUploadBufferToTexture& cmd
);
void preprocessCommand(
    VKCommandManifest& manifest, const CmdDownloadTextureToBuffer& cmd
);
void preprocessCommand(
    VKCommandManifest& manifest, const CmdBeginRenderPass& cmd
);
void preprocessCommand(
    VKCommandManifest& manifest, const CmdEndRenderPass& cmd
);

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdUploadBufferToTexture& cmd
);
Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdDownloadTextureToBuffer& cmd
);
Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdBeginRenderPass& cmd
);
Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdEndRenderPass& cmd
);

}  // namespace ignis::rhi