#include "ember/core/Core.h"

#include <fmt/format.h>

namespace ember::core {

std::string greet(const std::string &name) {
  return fmt::format("Hello, {}! Welcome to Ember.", name);
}

} // namespace ember::core
