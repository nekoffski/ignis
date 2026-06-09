#include "WavefrontLoader.hh"

namespace ignis::asset {

Result<render::MeshData> WavefrontLoader::load(const Path& path) && {
    if (auto err = loadFile(path); err) {
        log::error(
            "Failed to load OBJ file '{}': {}", path.str(), err->message()
        );
        return Error::unexpected(*err);
    }

    if (auto err = parseMesh(); err) {
        log::error(
            "Failed to parse OBJ file '{}': {}", path.str(), err->message()
        );
        return Error::unexpected(*err);
    }

    log::debug("Successfully loaded OBJ file '{}'", path.str());
    return m_meshDescription;
}

Opt<Error> WavefrontLoader::loadFile(const Path& path) {
    tinyobj::ObjReaderConfig cfg;
    cfg.triangulate = true;

    if (not reader.ParseFromFile(path.str(), cfg)) {
        if (not reader.Error().empty()) {
            return Error{
                Error::Code::ioError, "Failed to load OBJ file '{}': {}",
                path.str(), reader.Error()
            };
        }
        return Error{
            Error::Code::ioError, "Failed to load OBJ file '{}': {}",
            path.str(), "Unknown error"
        };
    }

    if (not reader.Warning().empty()) {
        log::warn(
            "Warnings while loading OBJ file '{}': {}", path.str(),
            reader.Warning()
        );
    }

    return Error::empty();
}

Opt<Error> WavefrontLoader::parseMesh() {
    const auto& shapes = reader.GetShapes();
    const auto& attributes = reader.GetAttrib();

    log::debug(
        "Parsing OBJ mesh with {} shapes, {} vertices, {} normals, {} "
        "texcoords",
        shapes.size(), attributes.vertices.size() / 3,
        attributes.normals.size() / 3, attributes.texcoords.size() / 2
    );

    for (const auto& shape : shapes) {
    }

    return Error::empty();
}

}  // namespace ignis::asset
