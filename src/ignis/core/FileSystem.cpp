#include "FileSystem.hh"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace ignis {

const Str& Path::str() const { return m_path; }

bool Path::isFile() const { return fs::is_regular_file(m_path); }

bool Path::isDirectory() const { return fs::is_directory(m_path); }

Path Path::parent() const {
    auto parentPath = fs::path(m_path).parent_path();
    return Path{parentPath.string()};
}

Path Path::join(const Path& base, const Path& relative) {
    return Path{fs::path(base.str()) / fs::path(relative.str())};
}

bool Path::endsWith(const Str& suffix) const {
    if (suffix.size() > m_path.size()) [[unlikely]] {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), m_path.rbegin());
}

Opt<Str> Path::extension() const {
    auto ext = fs::path(m_path).extension();
    if (ext.empty()) {
        return std::nullopt;
    }
    return ext.string();
}

Opt<Str> Path::filename() const {
    auto fname = fs::path(m_path).filename();
    if (fname.empty()) {
        return std::nullopt;
    }
    return fname.string();
}

void Path::append(const Str& suffix) { m_path += suffix; }

File::File(const Path& path) : m_path(path) {}

const Path& File::path() const { return m_path; }

Opt<Error> File::append(const Str& content) {
    std::ofstream file(m_path.str(), std::ios::app);
    if (!file.is_open()) {
        return Error{Error::Code::ioError, "Failed to open file for appending"};
    }
    file << content;
    return Error::empty();
}

Opt<Error> File::write(const Str& content) {
    std::ofstream file(m_path.str(), std::ios::trunc);
    if (!file.is_open()) {
        return Error{Error::Code::ioError, "Failed to open file for writing"};
    }
    file << content;
    return Error::empty();
}

Result<Str> File::read() const {
    std::ifstream file(m_path.str());
    if (!file.is_open()) {
        return Error::unexpected(
            Error::Code::ioError, "Failed to open file for reading"
        );
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

Result<std::vector<Str>> File::readLines() const {
    std::ifstream file(m_path.str());
    if (!file.is_open()) {
        return Error::unexpected(
            Error::Code::ioError, "Failed to open file for reading"
        );
    }
    std::vector<Str> lines;
    Str line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

Result<std::vector<u32>> File::readBinary() const {
    std::ifstream file(m_path.str(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return Error::unexpected(
            Error::Code::ioError, "Failed to open file '{}' for reading",
            m_path.str()
        );
    }
    auto size = file.tellg();
    if (size % sizeof(u32) != 0) {
        return Error::unexpected(
            Error::Code::invalidArgument,
            "File '{}' size is not a multiple of 4 bytes", m_path.str()
        );
    }
    file.seekg(0);
    std::vector<u32> buffer(static_cast<std::size_t>(size) / sizeof(u32));
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

Opt<Error> File::remove() {
    std::error_code ec;
    fs::remove(m_path.str(), ec);
    if (ec) {
        return Error{Error::Code::ioError, "Failed to remove file"};
    }
    return Error::empty();
}

}  // namespace ignis
