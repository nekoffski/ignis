#pragma once

#include "TinyObj.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Error.hh"
#include "ignis/core/FileSystem.hh"
#include "ignis/render/Mesh.hh"

namespace ignis::asset {

class WavefrontLoader : public NonCopyable, public NonMovable {
   public:
    WavefrontLoader() = default;

    Result<render::MeshData> load(const Path& path) &&;

   private:
    Opt<Error> loadFile(const Path& path);
    Opt<Error> parseMesh();

    tinyobj::ObjReader reader;
    render::MeshData m_meshDescription;
};

}  // namespace ignis::asset
