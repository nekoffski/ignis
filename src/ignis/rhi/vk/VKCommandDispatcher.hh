#pragma once

#include <span>

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Error.hh"
#include "ignis/rhi/Command.hh"

namespace ignis::rhi {

class VKDevice;

class VKCommandDispatcher : public NonCopyable, public NonMovable {
    class Visitor : public NonCopyable, public NonMovable {
       public:
        explicit Visitor(VKDevice& device, VkCommandBuffer cmdBuffer,
                         Queue targetQueue);

        Opt<Error> operator()(const CmdUploadBufferToTexture& cmd);
        Opt<Error> operator()(const CmdDownloadTextureToBuffer& cmd);
        Opt<Error> operator()(const CmdBeginRenderPass& cmd);
        Opt<Error> operator()(const CmdEndRenderPass& cmd);

       private:
        VKDevice& m_device;
        VkCommandBuffer m_cmdBuffer;
        Queue m_targetQueue;
    };

   public:
    explicit VKCommandDispatcher(VKDevice& device, VkCommandBuffer cmdBuffer,
                                 Queue targetQueue);

    Opt<Error> dispatch(const Command& command);
    Opt<Error> dispatch(std::span<const Command> commands);

   private:
    Visitor m_visitor;
};

}  // namespace ignis::rhi
