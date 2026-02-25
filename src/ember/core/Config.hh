#pragma once

#include <unordered_map>

#include "Core.hh"
#include "Singleton.hh"

namespace ember {

class Config : public Singleton<Config> {
   public:
    //     struct Directories {
    //         std::string home;
    //         std::string etc;
    //         std::string bin;
    //         std::string log;
    //     };

    //     struct Version {
    //         u16 major;
    //         u16 minor;
    //         u16 patch;
    //     };

    //     struct Daemon {
    //         u16 port;
    //         std::string pidFile;
    //         std::string binaryPath;
    //     };

    //     struct Logging {
    //         std::string level;
    //         std::string file;
    //     };

    void loadFromFile(const std::string& path);

    //     const Directories& directories() const;
    //     const Daemon& daemon() const;
    //     const Logging& logging() const;
    //     const Version& version() const;

   private:
    //     Directories m_directories;
    //     Daemon m_daemon;
    //     Logging m_logging;
    //     Version m_version;

    //     void readHomeDir();
    //     void parseConfig();
};

}  // namespace ember