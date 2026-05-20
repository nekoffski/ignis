#pragma once

#include <string>
#include <toml++/toml.hpp>
#include <unordered_map>
#include <vector>

#include "Core.hh"
#include "FileSystem.hh"

namespace ignis {

class Config {
   public:
    struct Renderer {
        enum class Backend {
            vulkan,
        };

        Backend backend{Backend::vulkan};
        u64 maxTextures{128u};
        u64 maxBuffers{128u};
        u64 maxRenderGraphs{128u};
        u64 maxPipelines{128u};
        u64 maxShaders{128u};
        u64 maxRenderPasses{128u};
        u64 maxBindGroups{128u};
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

    struct Asset {
        Path path;
    };

    const Renderer& renderer() const;
    const Vulkan& vulkan() const;
    const Version& version() const;
    const Core& core() const;
    const Asset& asset() const;

    static Config fromFile(const Path& path);

   private:
    void parseFields(const Path& path);

    Renderer m_renderer;
    Version m_version;
    std::optional<Vulkan> m_vulkan;
    Core m_core;
    Asset m_asset;
};

}  // namespace ignis
