#include "Workload.hh"

namespace ignis {

Workload::Workload(Queue targetQueue) : m_targetQueue(targetQueue) {}

Queue Workload::targetQueue() const { return m_targetQueue; }

std::span<const Command> Workload::commands() const { return m_commands; }

}  // namespace ignis
