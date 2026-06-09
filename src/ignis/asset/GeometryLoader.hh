#pragma once

#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Error.hh"
#include "ignis/render/Mesh.hh"

namespace ignis::asset {

class GeometryLoader : public NonCopyable, public NonMovable {
   public:
    explicit GeometryLoader();
    explicit GeometryLoader(const Config& config);

    Result<render::MeshData> load(const Path& path) &&;

   private:
    Config m_config;
};

}  // namespace ignis::asset
