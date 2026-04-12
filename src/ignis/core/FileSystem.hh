#pragma once

#include <string>
#include <vector>

#include "Error.hh"
#include "Singleton.hh"

namespace ignis {

class Path {
   public:
    template <typename... Args>
        requires std::constructible_from<std::string, Args...>
    Path(Args&&... args) : m_path(std::forward<Args>(args)...) {}

    const std::string& str() const;

    bool isFile() const;
    bool isDirectory() const;

   private:
    std::string m_path;
};

class File {
   public:
    explicit File(const Path& path);

    const Path& path() const;

    OError append(const std::string& content);
    OError write(const std::string& content);
    Result<std::string> read() const;
    Result<std::vector<std::string>> readLines() const;

    OError remove();

   private:
    Path m_path;
};

class Directory {};

}  // namespace ignis
