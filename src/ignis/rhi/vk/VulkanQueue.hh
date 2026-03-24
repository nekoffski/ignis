#pragma once

#include <unordered_map>

#include "Vulkan.hh"
#include "ignis/rhi/Queue.hh"

namespace ignis {

using VulkanQueueSet = std::unordered_map<Queue, VkQueue>;

}
