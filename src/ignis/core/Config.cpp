#include "Config.hh"

#include <toml++/toml.hpp>

#include "FileSystem.hh"
#include "Log.hh"

namespace ignis {

namespace {

Config::Renderer::Backend parseBackend(const std::string& backendStr) {
    if (backendStr == "vulkan")
        return Config::Renderer::Backend::vulkan;
    else
        log::panic("Unknown renderer backend: {}", backendStr);
}

class Reader : public NonCopyable, public NonMovable {
   public:
    explicit Reader(const toml::table& tbl) : m_tbl(tbl) {}

    template <typename T>
    T read(const std::string& stanza, const std::string& key) const {
        log::expect(m_tbl.contains(stanza),
                    "Config file must contain a [{}] table", stanza);
        auto subTable = m_tbl.get(stanza)->as_table();
        log::expect(subTable, "Config file must contain a [{}] table", stanza);
        log::expect(subTable->contains(key),
                    "Config file must contain a {} field in the [{}] table",
                    key, stanza);

        auto raw = subTable->get(key);

        if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            log::expect(raw->is_array(),
                        "Config file field {}.{} must be an array", stanza,
                        key);
            std::vector<std::string> result;
            for (const auto& item : *raw->as_array()) {
                log::expect(
                    item.is_string(),
                    "Config file field {}.{} must be an array of strings",
                    stanza, key);
                result.push_back(item.value<std::string>().value_or(""));
            }
            log::debug("Config: read {}.{} = [ {} ]", stanza, key,
                       fmt::join(result, ","));
            return result;
        } else {
            auto v = raw->value<T>();
            log::expect(v.has_value(),
                        "Config file must contain a {} field in the [{}] table",
                        key, stanza);
            log::debug("Config: read {}.{} = {}", stanza, key, *v);
            return *v;
        }
    }

   private:
    const toml::table& m_tbl;
};

}  // namespace

Config Config::fromFile(const Path& path) {
    Config cfg;

    log::expect(path.isFile(), "Config path {} does not exist or is not a file",
                path.str());

    try {
        cfg.parseFields(path);
    } catch (const toml::parse_error& e) {
        log::panic("Failed to parse config file: {}", e.what());
    } catch (const std::bad_optional_access& e) {
        log::panic("Failed to parse config file: {}", e.what());
    }

    return cfg;
}

using StrVec = std::vector<std::string>;

void Config::parseFields(const Path& path) {
    auto tbl = toml::parse_file(path.str());
    Reader r{tbl};

    m_version.major = r.read<u32>("version", "major");
    m_version.minor = r.read<u32>("version", "minor");
    m_version.patch = r.read<u32>("version", "patch");

    m_core.appName = r.read<std::string>("core", "appName");

    m_renderer.backend =
        parseBackend(r.read<std::string>("renderer", "backend"));
    m_renderer.maxTextures = r.read<u64>("renderer", "maxTextures");
    m_renderer.maxBuffers = r.read<u64>("renderer", "maxBuffers");
    m_renderer.maxRenderGraphs = r.read<u64>("renderer", "maxRenderGraphs");

    if (m_renderer.backend == Renderer::Backend::vulkan) {
        m_vulkan.emplace();

        m_vulkan->api = r.read<std::string>("vulkan", "apiVersion") == "v1.3"
                            ? Vulkan::Api::v1_3
                            : Vulkan::Api::v1_3;
        m_vulkan->extensions = r.read<StrVec>("vulkan", "extensions");
        m_vulkan->layers = r.read<StrVec>("vulkan", "layers");
    }
}

// -- getters

const Config::Renderer& Config::renderer() const { return m_renderer; }

const Config::Version& Config::version() const { return m_version; }

const Config::Vulkan& Config::vulkan() const {
    log::expect(m_vulkan.has_value(), "Vulkan config is not available");
    return m_vulkan.value();
}

const Config::Core& Config::core() const { return m_core; }

}  // namespace ignis
