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

    Path parent() const;

    static Path join(const Path& base, const Path& relative);

    bool endsWith(const std::string& suffix) const;
    void append(const std::string& suffix);

   private:
    std::string m_path;
};

class File {
   public:
    explicit File(const Path& path);

    const Path& path() const;

    Opt<Error> append(const std::string& content);
    Opt<Error> write(const std::string& content);
    Result<std::string> read() const;
    Result<std::vector<std::string>> readLines() const;
    Result<std::vector<u32>> readBinary() const;

    Opt<Error> remove();

   private:
    Path m_path;
};

class Directory {};

}  // namespace ignis
