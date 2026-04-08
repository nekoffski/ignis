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
    };

    explicit Error(Code code,
                   const std::string& message = "No details provided");

    Code code() const;
    const std::string& message() const;

    static std::unexpected<Error> unexpected(
        Code code, const std::string& message = "No details provided");

    static std::optional<Error> empty();

   private:
    Code m_code;
    std::string m_message;
};

template <typename T>
using Result = std::expected<T, Error>;

using OError = std::optional<Error>;

}  // namespace ignis
