#include "Core.hh"

#include "Random.hh"

namespace ignis {

Tag<Str> Tag<Str>::fromUuid() { return Tag<Str>{RandomEngine::get().uuid()}; }

}  // namespace ignis
