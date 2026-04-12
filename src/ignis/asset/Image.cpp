#include "Image.hh"

#include <stb_image_write.h>

namespace ignis {

namespace {

bool writeImpl(const Path& path, ImageFormat format,
               const ImageWriter::RawImageData& imageData) {
    const auto* pathStr = path.str().c_str();
    const auto* data = imageData.pixels;
    const i32 w = static_cast<i32>(imageData.width);
    const i32 h = static_cast<i32>(imageData.height);
    const i32 ch = static_cast<i32>(imageData.channels);

    int ok = 0;
    switch (format) {
        case ImageFormat::png:
            ok = stbi_write_png(pathStr, w, h, ch, data, w * ch);
            break;
        case ImageFormat::jpeg:
            ok = stbi_write_jpg(pathStr, w, h, ch, data, 90);
            break;
        case ImageFormat::bmp:
            ok = stbi_write_bmp(pathStr, w, h, ch, data);
            break;
        case ImageFormat::tga:
            ok = stbi_write_tga(pathStr, w, h, ch, data);
            break;
        default:
            return false;
    }

    return ok != 0;
}

}  // namespace

ImageWriter::ImageWriter(const Path& path) : m_path(path) {}

ImageWriter& ImageWriter::format(ImageFormat format) {
    m_format = format;
    return *this;
}

OError ImageWriter::write(const RawImageData& imageData) {
    if (m_format == ImageFormat::none)
        if (auto err = detectFormat(); err) return err;

    if (not writeImpl(m_path, m_format, imageData))
        return Error{Error::Code::ioError, "stb_image_write failed"};

    return Error::empty();
}

OError ImageWriter::detectFormat() {
    if (const auto& pathStr = m_path.str(); pathStr.ends_with(".png"))
        m_format = ImageFormat::png;
    else if (pathStr.ends_with(".jpg") || pathStr.ends_with(".jpeg"))
        m_format = ImageFormat::jpeg;
    else if (pathStr.ends_with(".bmp"))
        m_format = ImageFormat::bmp;
    else if (pathStr.ends_with(".tga"))
        m_format = ImageFormat::tga;
    else
        return Error{Error::Code::invalidArgument, "Unsupported image format"};

    return Error::empty();
}

}  // namespace ignis
