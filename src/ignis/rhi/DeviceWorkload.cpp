#include "DeviceWorkload.hh"

namespace ignis {

DeviceWorkload::DeviceWorkload(DeviceQueue targetQueue)
    : m_targetQueue(targetQueue) {
    log::expect(targetQueue != DeviceQueue::none, "Invalid target queue");
}

DeviceQueue DeviceWorkload::targetQueue() const { return m_targetQueue; }

std::span<const DeviceCommand> DeviceWorkload::commands() const {
    return m_commands;
}

void DeviceWorkload::addDependency(DeviceWorkloadReceipt receipt) {
    m_dependencies.push_back(receipt);
}

std::span<const DeviceWorkloadReceipt> DeviceWorkload::dependencies() const {
    return m_dependencies;
}

}  // namespace ignis
