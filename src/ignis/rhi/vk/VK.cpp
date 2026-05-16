#include "VK.hh"

namespace ignis::rhi {

VkCommandBufferUsageFlags toVk(BufferUsage usage) {
    return static_cast<VkCommandBufferUsageFlags>(usage);
}

VkMemoryPropertyFlags toVk(MemoryProperty memoryProperty) {
    return static_cast<VkMemoryPropertyFlags>(memoryProperty);
}

VkFormat toVk(Format format) {
    switch (format) {
        case Format::undefined:
            return VK_FORMAT_UNDEFINED;
        case Format::r8unorm:
            return VK_FORMAT_R8_UNORM;
        case Format::r8g8unorm:
            return VK_FORMAT_R8G8_UNORM;
        case Format::r8g8b8unorm:
            return VK_FORMAT_R8G8B8_UNORM;
        case Format::r8g8b8a8unorm:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::r32sfloat:
            return VK_FORMAT_R32_SFLOAT;
        case Format::r32g32sfloat:
            return VK_FORMAT_R32G32_SFLOAT;
        case Format::r32g32b32sfloat:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case Format::r32g32b32a32sfloat:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Format::r32sint:
            return VK_FORMAT_R32_SINT;
        case Format::r32uint:
            return VK_FORMAT_R32_UINT;
        default:
            log::error(
                "Unsupported texture format: {}", fmt::underlying(format)
            );
            return VK_FORMAT_UNDEFINED;
    }
}

VkImageTiling toVk(Tiling tiling) {
    switch (tiling) {
        case Tiling::optimal:
            return VK_IMAGE_TILING_OPTIMAL;
        case Tiling::linear:
            return VK_IMAGE_TILING_LINEAR;
        default:
            log::error(
                "Unsupported texture tiling: {}", fmt::underlying(tiling)
            );
            return VK_IMAGE_TILING_OPTIMAL;
    }
}

VkImageUsageFlags toVk(TextureUsage usage) {
    return static_cast<VkImageUsageFlags>(usage);
}

VkImageAspectFlags toVk(TextureAspect aspect) {
    return static_cast<VkImageAspectFlags>(aspect);
}

VkFilter toVk(Filter filter) { return static_cast<VkFilter>(filter); }

VkSamplerAddressMode toVk(Repeat repeat) {
    return static_cast<VkSamplerAddressMode>(repeat);
}

VkImageLayout toVk(Layout layout) {
    switch (layout) {
        case Layout::undefined:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case Layout::general:
            return VK_IMAGE_LAYOUT_GENERAL;
        case Layout::colorAttachment:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case Layout::depthStencilAttachment:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case Layout::depthStencilReadOnly:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case Layout::shaderReadOnly:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case Layout::transferSrc:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case Layout::transferDst:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case Layout::presentSrc:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        default:
            log::error("Unsupported image layout: {}", fmt::underlying(layout));
            return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

Format fromVk(VkFormat format) {
    switch (format) {
        case VK_FORMAT_UNDEFINED:
            return Format::undefined;
        case VK_FORMAT_R8_UNORM:
            return Format::r8unorm;
        case VK_FORMAT_R8G8_UNORM:
            return Format::r8g8unorm;
        case VK_FORMAT_R8G8B8_UNORM:
            return Format::r8g8b8unorm;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return Format::r8g8b8a8unorm;
        case VK_FORMAT_R32_SFLOAT:
            return Format::r32sfloat;
        case VK_FORMAT_R32G32_SFLOAT:
            return Format::r32g32sfloat;
        case VK_FORMAT_R32G32B32_SFLOAT:
            return Format::r32g32b32sfloat;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return Format::r32g32b32a32sfloat;
        case VK_FORMAT_R32_SINT:
            return Format::r32sint;
        case VK_FORMAT_R32_UINT:
            return Format::r32uint;
        default:
            log::error(
                "Unsupported Vulkan format: {}", fmt::underlying(format)
            );
            return Format::undefined;
    }
}

VkShaderStageFlags toVk(ShaderStageType stage) {
    VkShaderStageFlags flags = 0;
    if (checkFlag(stage, ShaderStageType::vertex))
        flags |= VK_SHADER_STAGE_VERTEX_BIT;
    if (checkFlag(stage, ShaderStageType::fragment))
        flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (checkFlag(stage, ShaderStageType::compute))
        flags |= VK_SHADER_STAGE_COMPUTE_BIT;
    if (checkFlag(stage, ShaderStageType::geometry))
        flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    return flags;
}

VkDescriptorType toVk(DescriptorType type) {
    switch (type) {
        case DescriptorType::uniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::storageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::sampledTexture:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::storageTexture:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case DescriptorType::sampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        default:
            log::error(
                "Unsupported descriptor type: {}", fmt::underlying(type)
            );
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
}

VkPolygonMode toVk(PolygonMode mode) {
    switch (mode) {
        case PolygonMode::fill:
            return VK_POLYGON_MODE_FILL;
        case PolygonMode::line:
            return VK_POLYGON_MODE_LINE;
        case PolygonMode::point:
            return VK_POLYGON_MODE_POINT;
        default:
            log::panic("unsupported PolygonMode: {}", fmt::underlying(mode));
    }
}

VkCullModeFlags toVk(CullMode mode) {
    switch (mode) {
        case CullMode::none:
            return VK_CULL_MODE_NONE;
        case CullMode::front:
            return VK_CULL_MODE_FRONT_BIT;
        case CullMode::back:
            return VK_CULL_MODE_BACK_BIT;
        default:
            log::panic("unsupported CullMode: {}", fmt::underlying(mode));
    }
}

VkFrontFace toVk(FrontFace face) {
    switch (face) {
        case FrontFace::clockwise:
            return VK_FRONT_FACE_CLOCKWISE;
        case FrontFace::counterClockwise:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        default:
            log::panic("unsupported FrontFace: {}", fmt::underlying(face));
    }
}

VkCompareOp toVk(CompareOp op) {
    switch (op) {
        case CompareOp::never:
            return VK_COMPARE_OP_NEVER;
        case CompareOp::less:
            return VK_COMPARE_OP_LESS;
        case CompareOp::equal:
            return VK_COMPARE_OP_EQUAL;
        case CompareOp::lessOrEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOp::greater:
            return VK_COMPARE_OP_GREATER;
        case CompareOp::notEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::greaterOrEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::always:
            return VK_COMPARE_OP_ALWAYS;
        default:
            log::panic("unsupported CompareOp: {}", fmt::underlying(op));
    }
}

VkBlendFactor toVk(BlendFactor factor) {
    switch (factor) {
        case BlendFactor::zero:
            return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::one:
            return VK_BLEND_FACTOR_ONE;
        case BlendFactor::srcColor:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::oneMinusSrcColor:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor::dstColor:
            return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor::oneMinusDstColor:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor::srcAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::oneMinusSrcAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::dstAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor::oneMinusDstAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        default:
            log::panic("unsupported BlendFactor: {}", fmt::underlying(factor));
    }
}

VkBlendOp toVk(BlendOp op) {
    switch (op) {
        case BlendOp::add:
            return VK_BLEND_OP_ADD;
        case BlendOp::subtract:
            return VK_BLEND_OP_SUBTRACT;
        case BlendOp::reverseSubtract:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOp::min:
            return VK_BLEND_OP_MIN;
        case BlendOp::max:
            return VK_BLEND_OP_MAX;
        default:
            log::panic("unsupported BlendOp: {}", fmt::underlying(op));
    }
}

std::string toString(VkResult result, bool extended) {
    switch (result) {
        default:
        case VK_SUCCESS:
            return !extended ? "VK_SUCCESS"
                             : "VK_SUCCESS Command successfully completed";
        case VK_NOT_READY:
            return !extended
                       ? "VK_NOT_READY"
                       : "VK_NOT_READY A fence or query has not yet completed";
        case VK_TIMEOUT:
            return !extended ? "VK_TIMEOUT"
                             : "VK_TIMEOUT A wait operation has not completed "
                               "in the specified time";
        case VK_EVENT_SET:
            return !extended ? "VK_EVENT_SET"
                             : "VK_EVENT_SET An event is signaled";
        case VK_EVENT_RESET:
            return !extended ? "VK_EVENT_RESET"
                             : "VK_EVENT_RESET An event is unsignaled";
        case VK_INCOMPLETE:
            return !extended ? "VK_INCOMPLETE"
                             : "VK_INCOMPLETE A return array was too small for "
                               "the result";
        case VK_SUBOPTIMAL_KHR:
            return !extended
                       ? "VK_SUBOPTIMAL_KHR"
                       : "VK_SUBOPTIMAL_KHR A swapchain no longer matches the "
                         "surface properties exactly, but can still be used to "
                         "present to the surface successfully.";
        case VK_THREAD_IDLE_KHR:
            return !extended ? "VK_THREAD_IDLE_KHR"
                             : "VK_THREAD_IDLE_KHR A deferred operation is not "
                               "complete but there is currently no work for "
                               "this thread to do at the time of this call.";
        case VK_THREAD_DONE_KHR:
            return !extended ? "VK_THREAD_DONE_KHR"
                             : "VK_THREAD_DONE_KHR A deferred operation is not "
                               "complete but there is no work remaining to "
                               "assign to additional threads.";
        case VK_OPERATION_DEFERRED_KHR:
            return !extended ? "VK_OPERATION_DEFERRED_KHR"
                             : "VK_OPERATION_DEFERRED_KHR A deferred operation "
                               "was requested and at least some of the work "
                               "was deferred.";
        case VK_OPERATION_NOT_DEFERRED_KHR:
            return !extended
                       ? "VK_OPERATION_NOT_DEFERRED_KHR"
                       : "VK_OPERATION_NOT_DEFERRED_KHR A deferred operation "
                         "was requested and no operations were deferred.";
        case VK_PIPELINE_COMPILE_REQUIRED_EXT:
            return !extended ? "VK_PIPELINE_COMPILE_REQUIRED_EXT"
                             : "VK_PIPELINE_COMPILE_REQUIRED_EXT A requested "
                               "pipeline creation would have required "
                               "compilation, but the application requested "
                               "compilation to not be performed.";

        // Error codes
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return !extended ? "VK_ERROR_OUT_OF_HOST_MEMORY"
                             : "VK_ERROR_OUT_OF_HOST_MEMORY A host memory "
                               "allocation has failed.";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return !extended ? "VK_ERROR_OUT_OF_DEVICE_MEMORY"
                             : "VK_ERROR_OUT_OF_DEVICE_MEMORY A device memory "
                               "allocation has failed.";
        case VK_ERROR_INITIALIZATION_FAILED:
            return !extended ? "VK_ERROR_INITIALIZATION_FAILED"
                             : "VK_ERROR_INITIALIZATION_FAILED Initialization "
                               "of an object could not be completed for "
                               "implementation-specific reasons.";
        case VK_ERROR_DEVICE_LOST:
            return !extended ? "VK_ERROR_DEVICE_LOST"
                             : "VK_ERROR_DEVICE_LOST The logical or physical "
                               "device has been lost. See Lost Device";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return !extended ? "VK_ERROR_MEMORY_MAP_FAILED"
                             : "VK_ERROR_MEMORY_MAP_FAILED Mapping of a memory "
                               "object has failed.";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return !extended ? "VK_ERROR_LAYER_NOT_PRESENT"
                             : "VK_ERROR_LAYER_NOT_PRESENT A requested layer "
                               "is not present or could not be loaded.";

        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return !extended ? "VK_ERROR_EXTENSION_NOT_PRESENT"
                             : "VK_ERROR_EXTENSION_NOT_PRESENT A requested "
                               "extension is not supported.";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return !extended ? "VK_ERROR_FEATURE_NOT_PRESENT"
                             : "VK_ERROR_FEATURE_NOT_PRESENT A requested "
                               "feature is not supported.";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return !extended ? "VK_ERROR_INCOMPATIBLE_DRIVER"
                             : "VK_ERROR_INCOMPATIBLE_DRIVER The requested "
                               "version of Vulkan is not supported by the "
                               "driver or is otherwise incompatible for "
                               "implementation-specific reasons.";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return !extended ? "VK_ERROR_TOO_MANY_OBJECTS"
                             : "VK_ERROR_TOO_MANY_OBJECTS Too many objects of "
                               "the type have already been created.";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return !extended ? "VK_ERROR_FORMAT_NOT_SUPPORTED"
                             : "VK_ERROR_FORMAT_NOT_SUPPORTED A requested "
                               "format is not supported on this device.";
        case VK_ERROR_FRAGMENTED_POOL:
            return !extended
                       ? "VK_ERROR_FRAGMENTED_POOL"
                       : "VK_ERROR_FRAGMENTED_POOL A pool allocation has "
                         "failed due to fragmentation of the pool’s memory. "
                         "This must only be returned if no attempt to allocate "
                         "host or device memory was made to accommodate the "
                         "new allocation. This should be returned in "
                         "preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only "
                         "if the implementation is certain that the pool "
                         "allocation failure was due to fragmentation.";
        case VK_ERROR_SURFACE_LOST_KHR:
            return !extended ? "VK_ERROR_SURFACE_LOST_KHR"
                             : "VK_ERROR_SURFACE_LOST_KHR A surface is no "
                               "longer available.";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return !extended
                       ? "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"
                       : "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR The requested "
                         "window is already in use by Vulkan or another API in "
                         "a manner which prevents it from being used again.";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return !extended
                       ? "VK_ERROR_OUT_OF_DATE_KHR"
                       : "VK_ERROR_OUT_OF_DATE_KHR A surface has changed in "
                         "such a way that it is no longer compatible with the "
                         "swapchain, and further presentation requests using "
                         "the swapchain will fail. Applications must query the "
                         "new surface properties and recreate their swapchain "
                         "if they wish to continue presenting to the surface.";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return !extended ? "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR"
                             : "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR The display "
                               "used by a swapchain does not use the same "
                               "presentable image layout, or is incompatible "
                               "in a way that prevents sharing an image.";
        case VK_ERROR_INVALID_SHADER_NV:
            return !extended ? "VK_ERROR_INVALID_SHADER_NV"
                             : "VK_ERROR_INVALID_SHADER_NV One or more shaders "
                               "failed to compile or link. More details are "
                               "reported back to the application via "
                               "VK_EXT_debug_report if enabled.";
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            return !extended
                       ? "VK_ERROR_OUT_OF_POOL_MEMORY"
                       : "VK_ERROR_OUT_OF_POOL_MEMORY A pool memory allocation "
                         "has failed. This must only be returned if no attempt "
                         "to allocate host or device memory was made to "
                         "accommodate the new allocation. If the failure was "
                         "definitely due to fragmentation of the pool, "
                         "VK_ERROR_FRAGMENTED_POOL should be returned instead.";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            return !extended
                       ? "VK_ERROR_INVALID_EXTERNAL_HANDLE"
                       : "VK_ERROR_INVALID_EXTERNAL_HANDLE An external handle "
                         "is not a valid handle of the specified type.";
        case VK_ERROR_FRAGMENTATION:
            return !extended ? "VK_ERROR_FRAGMENTATION"
                             : "VK_ERROR_FRAGMENTATION A descriptor pool "
                               "creation has failed due to fragmentation.";
        case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
            return !extended ? "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT"
                             : "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT A buffer "
                               "creation failed because the requested address "
                               "is not available.";
        // NOTE: Same as above
        // case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        //    return !extended ? "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS"
        //    :"VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS A buffer creation or
        //    memory allocation failed because the requested address is not
        //    available. A shader group handle assignment failed because the
        //    requested shader group handle information is no longer valid.";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            return !extended ? "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT"
                             : "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT "
                               "An operation on a swapchain created with "
                               "VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_"
                               "CONTROLLED_EXT failed as it did not have "
                               "exlusive full-screen access. This may occur "
                               "due to implementation-dependent reasons, "
                               "outside of the application’s control.";
        case VK_ERROR_UNKNOWN:
            return !extended
                       ? "VK_ERROR_UNKNOWN"
                       : "VK_ERROR_UNKNOWN An unknown error has occurred; "
                         "either the application has provided invalid input, "
                         "or an implementation failure has occurred.";
    }
}

}  // namespace ignis::rhi