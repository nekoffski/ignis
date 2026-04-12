#pragma once

#include <span>

#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/FileSystem.hh"
#include "ignis/core/Math.hh"

namespace ignis {

enum class ImageFormat {
    none,
    png,
    jpeg,
    bmp,
    tga,
};

class ImageWriter : public NonCopyable, public NonMovable {
   public:
    struct RawImageData {
        const u8* pixels;
        u32 width;
        u32 height;
        u32 channels;
    };

    explicit ImageWriter(const Path& path);
    ImageWriter& format(ImageFormat format);

    OError write(const RawImageData& imageData);

   private:
    Path m_path;
    ImageFormat m_format{ImageFormat::none};

    OError detectFormat();
};

}  // namespace ignis
