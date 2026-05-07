#include "FileSystem.hh"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace ignis {

const std::string& Path::str() const { return m_path; }

bool Path::isFile() const { return fs::is_regular_file(m_path); }

bool Path::isDirectory() const { return fs::is_directory(m_path); }

File::File(const Path& path) : m_path(path) {}

const Path& File::path() const { return m_path; }

Opt<Error> File::append(const std::string& content) {
    std::ofstream file(m_path.str(), std::ios::app);
    if (!file.is_open())
        return Error{Error::Code::noError, "Failed to open file for appending"};
    file << content;
    return Error::empty();
}

Opt<Error> File::write(const std::string& content) {
    std::ofstream file(m_path.str(), std::ios::trunc);
    if (!file.is_open())
        return Error{Error::Code::noError, "Failed to open file for writing"};
    file << content;
    return Error::empty();
}

Result<std::string> File::read() const {
    std::ifstream file(m_path.str());
    if (!file.is_open()) {
        return Error::unexpected(
            Error::Code::noError, "Failed to open file for reading"
        );
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

Result<std::vector<std::string>> File::readLines() const {
    std::ifstream file(m_path.str());
    if (!file.is_open()) {
        return Error::unexpected(
            Error::Code::noError, "Failed to open file for reading"
        );
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) lines.push_back(line);
    return lines;
}

Opt<Error> File::remove() {
    std::error_code ec;
    fs::remove(m_path.str(), ec);
    if (ec) return Error{Error::Code::noError, "Failed to remove file"};
    return Error::empty();
}

}  // namespace ignis
