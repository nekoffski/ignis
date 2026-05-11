#pragma once

#include "VKCommandDispatcher.hh"

namespace ignis::rhi {

Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdUploadBufferToTexture& cmd
);
Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdDownloadTextureToBuffer& cmd
);
Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdBeginRenderPass& cmd
);
Opt<Error> preprocessCommand(
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