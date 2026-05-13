#pragma once

#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Error.hh"
#include "ignis/rhi/Shader.hh"

namespace ignis::asset {

class ShaderParser : public NonCopyable, public NonMovable {
    struct ShaderFile;

    inline static std::string shaderFileExtension = ".igshader";

   public:
    explicit ShaderParser();
    explicit ShaderParser(const Config& config);

    Result<rhi::ShaderDescription> parseFile(const Path& path) &&;

   private:
    Result<rhi::ShaderDescription> parseFile(const ShaderFile& shaderFile);

    Config m_config;
};

}  // namespace ignis::asset
