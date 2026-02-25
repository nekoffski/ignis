#pragma once

#include <expected>
#include <optional>
#include <string>

namespace ember {

class Error {
   public:
    enum class Code {
        noError = 0,
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

}  // namespace ember
