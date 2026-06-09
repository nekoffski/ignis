#pragma once

#include <string>
#include <vector>

#include "Core.hh"
#include "Error.hh"
#include "Singleton.hh"

namespace ignis {

class Path {
   public:
    template <typename... Args>
        requires std::constructible_from<Str, Args...>
    Path(Args&&... args) : m_path(std::forward<Args>(args)...) {}

    const Str& str() const;

    bool isFile() const;
    bool isDirectory() const;

    Path parent() const;

    Opt<Str> extension() const;
    Opt<Str> filename() const;

    static Path join(const Path& base, const Path& relative);

    bool endsWith(const Str& suffix) const;
    void append(const Str& suffix);

   private:
    Str m_path;
};

class File {
   public:
    explicit File(const Path& path);

    const Path& path() const;

    Opt<Error> append(const Str& content);
    Opt<Error> write(const Str& content);
    Result<Str> read() const;
    Result<std::vector<Str>> readLines() const;
    Result<std::vector<u32>> readBinary() const;

    Opt<Error> remove();

   private:
    Path m_path;
};

class Directory {};

}  // namespace ignis
