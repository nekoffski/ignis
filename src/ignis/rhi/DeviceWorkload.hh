#pragma once

#include <span>
#include <vector>

#include "DeviceCommand.hh"
#include "DeviceQueue.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Log.hh"

namespace ignis {

using DeviceWorkloadReceipt = u8;

class DeviceWorkload : public NonCopyable, public NonMovable {
   public:
    explicit DeviceWorkload(DeviceQueue targetQueue);

    template <CommandConcept Command>
    void enqueue(const Command& command) {
        log::expect(
            command.targetQueue() == m_targetQueue,
            "Command target queue does not match workload target queue");
        m_commands.push_back(command);
    }

    void addDependency(DeviceWorkloadReceipt receipt);
    std::span<const DeviceWorkloadReceipt> dependencies() const;

    DeviceQueue targetQueue() const;
    std::span<const DeviceCommand> commands() const;

   private:
    DeviceQueue m_targetQueue;
    std::vector<DeviceCommand> m_commands;
    std::vector<DeviceWorkloadReceipt> m_dependencies;
};

}  // namespace ignis
