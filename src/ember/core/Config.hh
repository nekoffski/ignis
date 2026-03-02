#pragma once

#include <string>
#include <toml++/toml.hpp>
#include <unordered_map>
#include <vector>

#include "Core.hh"
#include "Singleton.hh"

namespace ember {

class Config : public Singleton<Config> {
   public:
    struct Renderer {
        enum class Backend {
            vulkan,
        };

        Backend backend{Backend::vulkan};
    };

    struct Version {
        u32 major{0};
        u32 minor{0};
        u32 patch{0};
    };

    struct Core {
        std::string appName;
    };

    struct Vulkan {
        enum class Api { v1_3 };

        Api api{Api::v1_3};
        std::vector<std::string> extensions;
        std::vector<std::string> layers;
    };

    const Renderer& renderer() const;
    const Vulkan& vulkan() const;
    const Version& version() const;
    const Core& core() const;

    void loadFromFile(const std::string& path);

   private:
    void parseFields(const std::string& path);

    Renderer m_renderer;
    Version m_version;
    std::optional<Vulkan> m_vulkan;
    Core m_core;
};

inline Config& cfg() { return Config::get(); }

}  // namespace ember
