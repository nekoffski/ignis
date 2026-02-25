#pragma once

#include <string>
#include <vector>

#include "Error.hh"
#include "Singleton.hh"

namespace ember {

class FileSystem : public Singleton<FileSystem> {
   public:
    using Path = std::string;

    bool isFile(const FileSystem::Path& path) const;
    bool isDirectory(const FileSystem::Path& path) const;

    Result<std::string> readFile(const FileSystem::Path& path) const;
    Result<std::vector<std::string>> readLines(
        const FileSystem::Path& path) const;

    OError appendFile(const FileSystem::Path& path,
                      const std::string& content) const;
    OError writeFile(const FileSystem::Path& path,
                     const std::string& content) const;

    OError removeFile(const FileSystem::Path& path) const;
};

}  // namespace ember
