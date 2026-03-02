#include "Config.hh"

#include <toml++/toml.hpp>

#include "FileSystem.hh"
#include "Log.hh"

namespace ember {

namespace {

Config::Renderer::Backend parseBackend(const std::string& backendStr) {
    if (backendStr == "vulkan") {
        log::info("Renderer.backend = VULKAN");
        return Config::Renderer::Backend::vulkan;
    } else {
        log::panic("Unknown renderer backend: {}", backendStr);
    }
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

        if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            log::expect(subTable->get(key)->is_array(),
                        "Config file field [{}].{} must be an array", stanza,
                        key);
            std::vector<std::string> result;
            for (const auto& item : *subTable->get(key)->as_array()) {
                log::expect(
                    item.is_string(),
                    "Config file field [{}].{} must be an array of strings",
                    stanza, key);
                result.push_back(item.value<std::string>().value_or(""));
            }
            return result;
        } else {
            auto v = subTable->get(key)->value<T>();
            log::expect(v.has_value(),
                        "Config file must contain a {} field in the [{}] table",
                        key, stanza);
            return *v;
        }
    }

   private:
    const toml::table& m_tbl;
};

}  // namespace

void Config::loadFromFile(const std::string& path) {
    auto& fs = FileSystem::get();
    log::expect(fs.isFile(path),
                "Config path {} does not exist or is not a file", path);

    try {
        parseFields(path);
    } catch (const toml::parse_error& e) {
        log::panic("Failed to parse config file: {}", e.what());
    } catch (const std::bad_optional_access& e) {
        log::panic("Failed to parse config file: {}", e.what());
    }
}

void Config::parseFields(const std::string& path) {
    auto tbl = toml::parse_file(path);
    Reader r{tbl};

    m_version.major = r.read<u32>("version", "major");
    m_version.minor = r.read<u32>("version", "minor");
    m_version.patch = r.read<u32>("version", "patch");

    m_core.appName = r.read<std::string>("core", "appName");

    m_renderer.backend =
        parseBackend(r.read<std::string>("renderer", "backend"));

    if (m_renderer.backend == Renderer::Backend::vulkan) {
        m_vulkan.emplace();

        m_vulkan->api = r.read<std::string>("vulkan", "apiVersion") == "v1.3"
                            ? Vulkan::Api::v1_3
                            : Vulkan::Api::v1_3;
        m_vulkan->extensions =
            r.read<std::vector<std::string>>("vulkan", "extensions");
        m_vulkan->layers = r.read<std::vector<std::string>>("vulkan", "layers");
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

}  // namespace ember
