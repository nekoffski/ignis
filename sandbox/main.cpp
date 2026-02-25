#include <fmt/format.h>

#include "ember/core/Config.hh"
#include "ember/core/Log.hh"
#include "ember/rhi/RHI.hh"

int main(int argc, char** argv) {
    ember::log::init();
    ember::log::expect(argc > 1, "No config file path provided");
    ember::Config::get().loadFromFile(argv[1]);

    auto rhi = ember::RHI::create();

    return 0;
}
