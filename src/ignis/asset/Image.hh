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

    Opt<Error> write(const RawImageData& imageData);

   private:
    Path m_path;
    ImageFormat m_format{ImageFormat::none};

    Opt<Error> detectFormat();
};

struct ImageUtils {
    static std::vector<u8> chessboard(u32 width, u32 height, u32 channels, u32 squareSize);
};

}  // namespace ignis
