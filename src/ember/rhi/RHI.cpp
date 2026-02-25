#include "RHI.hh"

namespace ember {

std::unique_ptr<RHI> RHI::create() { return std::unique_ptr<RHI>(); }

}  // namespace ember
