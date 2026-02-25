#include "Error.hh"

namespace ember {

Error::Error(Code code, const std::string& message)
    : m_code(code), m_message(message) {}

Error::Code Error::code() const { return m_code; }

const std::string& Error::message() const { return m_message; }

std::unexpected<Error> Error::unexpected(Code code,
                                         const std::string& message) {
    return std::unexpected{Error{code, message}};
}

std::optional<Error> Error::empty() { return {}; }

}  // namespace ember
