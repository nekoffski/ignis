#pragma once

#include <span>
#include <vector>

#include "Command.hh"
#include "Queue.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Log.hh"

namespace ignis::rhi {

using WorkloadReceipt = TimelinePoint;

class Workload : public NonCopyable, public NonMovable {
   public:
    explicit Workload(Queue targetQueue);

    template <CommandConcept Cmd>
    void enqueue(const Cmd& command) {
        log::expect(
            command.targetQueue() == m_targetQueue,
            "Command target queue does not match workload target queue"
        );
        m_commands.push_back(command);
    }

    void addDependency(WorkloadReceipt receipt);
    std::span<const WorkloadReceipt> dependencies() const;

    Queue targetQueue() const;
    std::span<const Command> commands() const;

   private:
    Queue m_targetQueue;
    std::vector<Command> m_commands;
    std::vector<WorkloadReceipt> m_dependencies;
};

}  // namespace ignis::rhi
