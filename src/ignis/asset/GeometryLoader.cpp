#include "GeometryLoader.hh"

#include "geometry/WavefrontLoader.hh"
#include "ignis/core/ServiceLocator.hh"

namespace ignis::asset {

GeometryLoader::GeometryLoader()
    : GeometryLoader(ServiceLocator<Config>::get()) {}

GeometryLoader::GeometryLoader(const Config& config) : m_config(config) {}

Result<render::MeshData> GeometryLoader::load(const Path& path) && {
    const auto fullPath = Path::join(m_config.asset().path, path);

    if (not fullPath.isFile()) {
        return Error::unexpected(
            Error::Code::fileDoesNotExist, "Asset file does not exist: {}",
            fullPath.str()
        );
    }

    if (auto extension = fullPath.extension(); extension) {
        if (*extension == ".obj") {
            return WavefrontLoader{}.load(fullPath);
        }
    }

    return Error::unexpected(
        Error::Code::unrecognizedFileFormat,
        "Unrecognized file format for asset: {}", fullPath.str()
    );
}

}  // namespace ignis::asset
