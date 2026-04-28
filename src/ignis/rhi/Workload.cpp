#include "Workload.hh"

namespace ignis::rhi {

Workload::Workload(Queue targetQueue) : m_targetQueue(targetQueue) {
    log::expect(targetQueue != Queue::none, "Invalid target queue");
}

Queue Workload::targetQueue() const { return m_targetQueue; }

std::span<const Command> Workload::commands() const { return m_commands; }

void Workload::addDependency(WorkloadReceipt receipt) {
    m_dependencies.push_back(receipt);
}

std::span<const WorkloadReceipt> Workload::dependencies() const {
    return m_dependencies;
}

}  // namespace ignis::rhi
