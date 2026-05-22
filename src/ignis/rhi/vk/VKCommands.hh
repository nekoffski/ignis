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
Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdBindPipeline& cmd
);
Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdBindBindGroup& cmd
);
Opt<Error> preprocessCommand(VKCommandManifest& manifest, const CmdDraw& cmd);
Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdSetViewport& cmd
);
Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdSetScissor& cmd
);
Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdDrawIndexed& cmd
);
Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdBindVertexBuffer& cmd
);
Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdBindIndexBuffer& cmd
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
Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdBindPipeline& cmd
);
Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdBindBindGroup& cmd
);
Opt<Error> recordCommand(const VKCommandContext& ctx, const CmdDraw& cmd);
Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdSetViewport& cmd
);
Opt<Error> recordCommand(const VKCommandContext& ctx, const CmdSetScissor& cmd);
Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdDrawIndexed& cmd
);
Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdBindVertexBuffer& cmd
);
Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdBindIndexBuffer& cmd
);

}  // namespace ignis::rhi