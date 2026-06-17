#include "ShaderParser.hh"

#include <spirv_reflect.h>
#include <toml++/toml.h>

#include <algorithm>

#include "ignis/core/Scope.hh"
#include "ignis/core/ServiceLocator.hh"

namespace ignis::asset {

struct ShaderParser::ShaderFile {
    std::unordered_map<rhi::ShaderStageType, std::vector<u32>> stages;
    rhi::ShaderStageType stagesMask{rhi::ShaderStageType::none};

    static Result<ShaderFile> load(const Path& path);
    static Opt<Error> parseEntry(
        toml::v3::node& node, ShaderFile& f, const Path& base
    );
    static Opt<Error> addStage(
        const Path& base, const std::string& stagePath,
        rhi::ShaderStageType stage, ShaderFile& f
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

static Opt<rhi::DescriptorType> toDescriptorType(SpvReflectDescriptorType type
) {
    switch (type) {
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            return rhi::DescriptorType::uniformBuffer;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            return rhi::DescriptorType::storageBuffer;
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return rhi::DescriptorType::sampledTexture;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return rhi::DescriptorType::storageTexture;
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
            return rhi::DescriptorType::sampler;
        default:
            return std::nullopt;
    }
}

static rhi::Format toFormat(SpvReflectFormat fmt) {
    switch (fmt) {
        case SPV_REFLECT_FORMAT_R32_SFLOAT:
            return rhi::Format::r32sfloat;
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
            return rhi::Format::r32g32sfloat;
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
            return rhi::Format::r32g32b32sfloat;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
            return rhi::Format::r32g32b32a32sfloat;
        case SPV_REFLECT_FORMAT_R32_SINT:
            return rhi::Format::r32sint;
        case SPV_REFLECT_FORMAT_R32_UINT:
            return rhi::Format::r32uint;
        default:
            return rhi::Format::undefined;
    }
}

static void reflectBindings(
    const SpvReflectShaderModule& module, rhi::ShaderStageType stageType,
    rhi::ShaderDescription& desc, std::unordered_map<u64, u32>& bindingIndex
) {
    u32 count = 0;
    spvReflectEnumerateDescriptorBindings(&module, &count, nullptr);
    std::vector<SpvReflectDescriptorBinding*> bindings(count);
    spvReflectEnumerateDescriptorBindings(&module, &count, bindings.data());

    for (auto* b : bindings) {
        auto descType = toDescriptorType(b->descriptor_type);
        if (not descType) {
            log::warn(
                "Unsupported descriptor type {} in stage '{}', skipping",
                static_cast<int>(b->descriptor_type),
                shaderStageToString(stageType)
            );
            continue;
        }
        u64 key = (static_cast<u64>(b->set) << 32) | b->binding;
        if (auto it = bindingIndex.find(key); it != bindingIndex.end()) {
            desc.bindings[it->second].stageFlags |= stageType;
        } else {
            bindingIndex[key] = static_cast<u32>(desc.bindings.size());
            desc.bindings.push_back({
                .set = b->set,
                .binding = b->binding,
                .type = *descType,
                .count = b->count,
                .stageFlags = stageType,
            });
        }
    }
}

static void reflectPushConstants(
    const SpvReflectShaderModule& module, rhi::ShaderStageType stageType,
    rhi::ShaderDescription& desc
) {
    u32 count = 0;
    spvReflectEnumeratePushConstantBlocks(&module, &count, nullptr);
    std::vector<SpvReflectBlockVariable*> pcs(count);
    spvReflectEnumeratePushConstantBlocks(&module, &count, pcs.data());

    for (auto* pc : pcs) {
        bool merged = false;
        for (auto& existing : desc.pushConstants) {
            if (existing.offset == pc->offset && existing.size == pc->size) {
                existing.stageFlags |= stageType;
                merged = true;
                break;
            }
        }
        if (not merged) {
            desc.pushConstants.push_back({
                .offset = pc->offset,
                .size = pc->size,
                .stageFlags = stageType,
            });
        }
    }
}

static void reflectVertexAttributes(
    const SpvReflectShaderModule& module, rhi::ShaderDescription& desc
) {
    u32 count = 0;
    spvReflectEnumerateInputVariables(&module, &count, nullptr);
    std::vector<SpvReflectInterfaceVariable*> inputs(count);
    spvReflectEnumerateInputVariables(&module, &count, inputs.data());

    for (auto* input : inputs) {
        if (input->built_in != -1) {
            continue;
        }
        desc.vertexAttributes.push_back({
            .location = input->location,
            .format = toFormat(input->format),
        });
    }

    std::sort(
        desc.vertexAttributes.begin(), desc.vertexAttributes.end(),
        [](const auto& a, const auto& b) { return a.location < b.location; }
    );
}

static Opt<Error> reflectStage(
    rhi::ShaderStageType stageType, const std::vector<u32>& spirv,
    rhi::ShaderDescription& desc, std::unordered_map<u64, u32>& bindingIndex
) {
    SpvReflectShaderModule module{};
    if (spvReflectCreateShaderModule(
            spirv.size() * sizeof(u32), spirv.data(), &module
        ) != SPV_REFLECT_RESULT_SUCCESS) {
        return Error{
            Error::Code::ioError,
            "Failed to create reflection module for stage '{}'",
            shaderStageToString(stageType),
        };
    }
    ON_SCOPE_EXIT { spvReflectDestroyShaderModule(&module); };

    reflectBindings(module, stageType, desc, bindingIndex);
    reflectPushConstants(module, stageType, desc);
    if (stageType == rhi::ShaderStageType::vertex) {
        reflectVertexAttributes(module, desc);
    }

    desc.stages.push_back({
        .stage = stageType,
        .entrypoint = module.entry_point_name,
        .spirv = spirv,
    });
    return Error::empty();
}

Result<rhi::ShaderDescription> ShaderParser::parseFile(
    const ShaderFile& shaderFile
) {
    rhi::ShaderDescription desc;
    desc.stagesMask = shaderFile.stagesMask;

    std::unordered_map<u64, u32> bindingIndex;

    for (const auto& [stageType, spirv] : shaderFile.stages) {
        log::debug("Reflecting stage '{}'", shaderStageToString(stageType));
        if (auto err = reflectStage(stageType, spirv, desc, bindingIndex);
            err) {
            return Error::unexpected(*err);
        }
    }

    log::debug("Shader description:\n{}", toString(desc));
    return desc;
}

static bool validateStages(
    rhi::ShaderStageType existingMask, rhi::ShaderStageType newStage
) {
    if (newStage == rhi::ShaderStageType::compute) {
        return existingMask == rhi::ShaderStageType::none;
    } else {
        return not checkFlag(existingMask, rhi::ShaderStageType::compute);
    }
}

Opt<Error> ShaderParser::ShaderFile::addStage(
    const Path& base, const std::string& stagePath, rhi::ShaderStageType stage,
    ShaderFile& f
) {
    log::debug(
        "Adding shader stage '{}' from path '{}'", shaderStageToString(stage),
        stagePath
    );

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

    if (auto sourceResult = File{sourcePath}.readBinary(); not sourceResult) {
        return Error{
            sourceResult.error().code(), "failed to read source file '{}': {}",
            sourcePath.str(), sourceResult.error().message()
        };
    } else {
        f.stages[stage] = std::move(*sourceResult);
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

    auto stage = rhi::shaderStageTypeFromString(*typeVal);
    if (stage == rhi::ShaderStageType::none) {
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
                "Failed to parse shader file '{}': {}", path.str(),
                err->message()
            );
            return Error::unexpected(*err);
        }
    }
    return file;
}

}  // namespace ignis::asset
