#pragma once

#include <unordered_map>

#include "VK.hh"
#include "ignis/rhi/Queue.hh"

namespace ignis {

using VKQueueSet = std::unordered_map<Queue, VkQueue>;

}
