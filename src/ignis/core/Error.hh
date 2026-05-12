#pragma once

#include <fmt/core.h>

#include <expected>
#include <optional>
#include <string>

namespace ignis {

class Error {
   public:
    enum class Code {
        noError = 0,
        poolFull = 1,
        queueSubmissionFailed = 2,
        invalidArgument = 3,
        resourceMissing = 4,
        ioError = 5,
        deviceQueueMismatch = 6,
        resourceInUse = 7,
        logicError = 8,
    };

    explicit Error(
        Code code, const std::string& message = "No details provided"
    );

    template <typename... Args>
    explicit Error(Code code, const std::string& fmt, Args&&... args)
        : m_code(code),
          m_message(fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...)
          ) {}

    Code code() const;
    const std::string& message() const;

    static std::unexpected<Error> unexpected(
        Code code, const std::string& message = "No details provided"
    );

    static std::unexpected<Error> unexpected(const Error& error);

    static std::optional<Error> empty();

   private:
    Code m_code;
    std::string m_message;
};

template <typename T>
using Result = std::expected<T, Error>;

}  // namespace ignis
