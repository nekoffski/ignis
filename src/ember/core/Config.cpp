#include "Config.hh"

// #include <boost/algorithm/string.hpp>
#include <toml++/toml.hpp>

#include "FileSystem.hh"
#include "Log.hh"

namespace ember {

void Config::loadFromFile(const std::string& path) {
    auto& fs = FileSystem::get();
    log::expect(fs.isDirectory(path), "Config path {} is not a directory",
                path);
}

// Config::Config() {
//     readHomeDir();

//     try {
//         parseConfig();
//     } catch (const toml::parse_error& e) {
//         throw ConfigError{Error::Code::configParsingError,
//                           "Failed to parse config file: {}", e.what()};
//     } catch (const std::bad_optional_access& e) {
//         throw ConfigError{Error::Code::configParsingError,
//                           "Failed to parse config file: {}", e.what()};
//     }
// }

// static Config::Version parseVersion(const std::string& versionFilePath) {
//     Config::Version version;
//     auto& fs = FileSystem::get();

//     if (not fs.isFile(versionFilePath)) {
//         throw ConfigError{Error::Code::versionFileNotFound,
//                           "Version file not found at {}", versionFilePath};
//     }

//     auto versionStr = fs.readFile(versionFilePath);

//     if (versionStr.empty()) {
//         throw ConfigError{Error::Code::invalidVersionFormat,
//                           "Version file is empty at {}", versionFilePath};
//     }

//     if (versionStr.back() == '\n') versionStr.pop_back();
//     if (versionStr.front() == 'v') versionStr.erase(0, 1);

//     std::vector<std::string> parts;
//     boost::algorithm::split(parts, versionStr, boost::is_any_of("."));

//     if (parts.size() != 3) {
//         throw ConfigError{Error::Code::invalidVersionFormat,
//                           "Version file has invalid format at {}",
//                           versionFilePath};
//     }

//     try {
//         version.major = std::stoul(parts[0]);
//         version.minor = std::stoul(parts[1]);
//         version.patch = std::stoul(parts[2]);
//     } catch (const std::exception& e) {
//         throw ConfigError{Error::Code::invalidVersionFormat,
//                           "Version file has invalid format at {}: {}",
//                           versionFilePath, e.what()};
//     }

//     return version;
// }

// const Config::Directories& Config::directories() const { return
// m_directories; }

// const Config::Daemon& Config::daemon() const { return m_daemon; }

// const Config::Logging& Config::logging() const { return m_logging; }

// const Config::Version& Config::version() const { return m_version; }

// void Config::readHomeDir() {
//     const auto homeDir = getEnv("ember_HOME");
//     if (not homeDir) {
//         throw ConfigError{Error::Code::misconfigured,
//                           "$ember_HOME environment variable is not set"};
//     }

//     m_directories.home = *homeDir;
//     m_directories.etc = m_directories.home + "/etc";
//     m_directories.bin = m_directories.home + "/bin";
//     m_directories.log = m_directories.home + "/var/log";
// }

// void Config::parseConfig() {
//     auto configPath = m_directories.etc + "/ember.toml";
//     auto tbl = toml::parse_file(configPath);

//     m_daemon.port = *tbl["daemon"]["port"].value<u16>();
//     m_daemon.pidFile = *tbl["daemon"]["pid_file"].value<std::string>();
//     m_logging.level = *tbl["logging"]["level"].value<std::string>();
//     m_logging.file = *tbl["logging"]["file"].value<std::string>();
//     m_daemon.binaryPath = m_directories.bin + "/emberd";
//     m_version = parseVersion(m_directories.etc + "/version");
// }

}  // namespace ember
