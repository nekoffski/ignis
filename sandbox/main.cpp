#include "ember/core/Core.h"

#include <fmt/format.h>
#include <iostream>

int main() {
  std::cout << ember::core::greet("World") << '\n';
  return 0;
}
