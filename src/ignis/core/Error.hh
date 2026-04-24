#pragma once

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
    };

    explicit Error(Code code,
                   const std::string& message = "No details provided");

    Code code() const;
    const std::string& message() const;

    static std::unexpected<Error> unexpected(
        Code code, const std::string& message = "No details provided");

    static std::unexpected<Error> unexpected(const Error& error);

    static std::optional<Error> empty();

   private:
    Code m_code;
    std::string m_message;
};

template <typename T>
using Result = std::expected<T, Error>;

}  // namespace ignis
