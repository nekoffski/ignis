#pragma once

#include <span>

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Error.hh"
#include "ignis/rhi/DeviceCommand.hh"

namespace ignis {

class VKDevice;

class VKCommandDispatcher : public NonCopyable, public NonMovable {
    class Visitor : public NonCopyable, public NonMovable {
       public:
        explicit Visitor(VKDevice& device, VkCommandBuffer cmdBuffer,
                         DeviceQueue targetQueue);

        Opt<Error> operator()(const CmdUploadBufferToTexture& cmd);
        Opt<Error> operator()(const CmdDownloadTextureToBuffer& cmd);

       private:
        VKDevice& m_device;
        VkCommandBuffer m_cmdBuffer;
        DeviceQueue m_targetQueue;
    };

   public:
    explicit VKCommandDispatcher(VKDevice& device, VkCommandBuffer cmdBuffer,
                                 DeviceQueue targetQueue);

    Opt<Error> dispatch(const DeviceCommand& command);
    Opt<Error> dispatch(std::span<const DeviceCommand> commands);

   private:
    Visitor m_visitor;
};

}  // namespace ignis
