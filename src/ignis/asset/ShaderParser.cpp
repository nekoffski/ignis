#include "ShaderParser.hh"

#include <toml++/toml.h>

#include "ignis/core/ServiceLocator.hh"

namespace ignis::asset {

struct ShaderParser::ShaderFile {
    struct Stage {
        rhi::ShaderStage stage;
        std::string source;
    };

    std::vector<Stage> stages;
    rhi::ShaderStage stagesMask{rhi::ShaderStage::none};

    static Result<ShaderFile> load(const Path& path);
    static Opt<Error> parseEntry(
        toml::v3::node& node, ShaderFile& f, const Path& base
    );
    static Opt<Error> addStage(
        const Path& base, const std::string& stagePath, rhi::ShaderStage stage,
        ShaderFile& f
    );
};

ShaderParser::ShaderParser() : ShaderParser(ServiceLocator<Config>::get()) {}

ShaderParser::ShaderParser(const Config& config) : m_config(config) {}

Result<rhi::ShaderDescription> ShaderParser::parseFile(const Path& path) && {
    auto filePath = Path::join(m_config.asset().path, path);

    if (not filePath.endsWith(shaderFileExtension)) {
        log::error(
            "Shader file {} does not have the expected extension {}",
            filePath.str(), shaderFileExtension
        );
        return Error::unexpected(
            Error::Code::invalidArgument,
            "Shader file does not have the expected extension"
        );
    }

    if (not filePath.isFile()) {
        log::error("Shader file {} does not exist", filePath.str());
        return Error::unexpected(
            Error::Code::fileDoesNotExist, "Shader file '{}' does not exist",
            filePath.str()
        );
    }

    if (auto shaderFile = ShaderFile::load(filePath); not shaderFile) {
        log::error(
            "Failed to load shader file {}: {}", filePath.str(),
            shaderFile.error().message()
        );
        return Error::unexpected(shaderFile.error());
    } else {
        return parseFile(*shaderFile);
    }
}

Result<rhi::ShaderDescription> ShaderParser::parseFile(
    const ShaderFile& shaderFile
) {
    return rhi::ShaderDescription{};
}

static bool validateStages(
    rhi::ShaderStage existingMask, rhi::ShaderStage newStage
) {
    if (newStage == rhi::ShaderStage::compute)
        return existingMask == rhi::ShaderStage::none;
    else
        return not checkFlag(existingMask, rhi::ShaderStage::compute);
}

Opt<Error> ShaderParser::ShaderFile::addStage(
    const Path& base, const std::string& stagePath, rhi::ShaderStage stage,
    ShaderFile& f
) {
    if (checkFlag(f.stagesMask, stage)) {
        return Error{
            Error::Code::invalidArgument, "Shader file has duplicate stage type"
        };
    }

    if (not validateStages(f.stagesMask, stage)) {
        return Error{
            Error::Code::invalidArgument,
            "Shader file cannot mix compute stage with other stages"
        };
    }

    f.stagesMask |= stage;

    auto sourcePath = Path::join(base, stagePath);

    if (not sourcePath.isFile()) {
        return Error{
            Error::Code::fileDoesNotExist, "source file '{}' does not exist",
            sourcePath.str()
        };
    }

    if (auto sourceResult = File{sourcePath}.read(); not sourceResult) {
        return Error{
            sourceResult.error().code(), "failed to read source file '{}': {}",
            sourcePath.str(), sourceResult.error().message()
        };
    } else {
        f.stages.emplace_back(stage, *sourceResult);
    }

    return Error::empty();
}

Opt<Error> ShaderParser::ShaderFile::parseEntry(
    toml::v3::node& entry, ShaderFile& f, const Path& base
) {
    auto* stageTable = entry.as_table();
    if (not stageTable) {
        return Error{
            Error::Code::invalidArgument, "[[stage]] entry is not a table"
        };
    }

    auto typeVal = (*stageTable)["type"].value<std::string>();
    if (not typeVal) {
        return Error{
            Error::Code::invalidArgument, "[[stage]] entry missing 'type'"
        };
    }

    auto stagePath = (*stageTable)["path"].value<std::string>();
    if (not stagePath) {
        return Error{
            Error::Code::invalidArgument, "[[stage]] entry missing 'path'"
        };
    }

    auto stage = rhi::shaderStageFromString(*typeVal);
    if (stage == rhi::ShaderStage::none) {
        return Error{
            Error::Code::invalidArgument, "[[stage]] entry has unknown type"
        };
    }

    return addStage(base, *stagePath, stage, f);
}

Result<ShaderParser::ShaderFile> ShaderParser::ShaderFile::load(const Path& path
) {
    toml::table table;
    try {
        table = toml::parse_file(path.str());
    } catch (const toml::parse_error& e) {
        return Error::unexpected(
            Error::Code::ioError, "Failed to parse shader file '{}': {}",
            path.str(), e.what()
        );
    }

    auto* stageArray = table["stage"].as_array();
    if (not stageArray or stageArray->empty()) {
        return Error::unexpected(
            Error::Code::invalidArgument,
            "Shader file '{}' contains no [[stage]] entries", path.str()
        );
    }

    ShaderFile file;
    const auto& parentPath = path.parent();

    for (auto& entry : *stageArray) {
        if (auto err = parseEntry(entry, file, parentPath); err) {
            log::error(
                "Failed to parse shader file {}: {}", path.str(), err->message()
            );
            return Error::unexpected(*err);
        }
    }
    return file;
}

}  // namespace ignis::asset
