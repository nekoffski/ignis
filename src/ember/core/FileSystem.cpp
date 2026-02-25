#include "FileSystem.hh"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace ember {

bool FileSystem::isFile(const FileSystem::Path& path) const {
    return fs::is_regular_file(path);
}

bool FileSystem::isDirectory(const FileSystem::Path& path) const {
    return fs::is_directory(path);
}

OError FileSystem::appendFile(const FileSystem::Path& path,
                              const std::string& content) const {
    std::ofstream file(path, std::ios::app);
    if (!file.is_open())
        return Error{Error::Code::noError, "Failed to open file for appending"};
    file << content;
    return Error::empty();
}

OError FileSystem::writeFile(const FileSystem::Path& path,
                             const std::string& content) const {
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open())
        return Error{Error::Code::noError, "Failed to open file for writing"};
    file << content;
    return Error::empty();
}

Result<std::string> FileSystem::readFile(const FileSystem::Path& path) const {
    std::ifstream file(path);
    if (!file.is_open()) {
        return Error::unexpected(Error::Code::noError,
                                 "Failed to open file for reading");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

Result<std::vector<std::string>> FileSystem::readLines(
    const FileSystem::Path& path) const {
    std::ifstream file(path);
    if (!file.is_open()) {
        return Error::unexpected(Error::Code::noError,
                                 "Failed to open file for reading");
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) lines.push_back(line);
    return lines;
}

OError FileSystem::removeFile(const FileSystem::Path& path) const {
    std::error_code ec;
    fs::remove(path, ec);
    if (ec) return Error{Error::Code::noError, "Failed to remove file"};
    return Error::empty();
}

}  // namespace ember
