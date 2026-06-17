module;
#include <utility>
export module Texture;
import Types;
import Image;
import Color;
import Vectors;

export struct Texture {
private:
    Image img;
public:
    explicit Texture(Image img) : img(move(img)) {}
    explicit Texture(const Int32 width, const Int32 height) : img(width, height) {}
    explicit Texture(const String& path) : img(Image::fromFile(path)) {}

    [[nodiscard]] Int32 getWidth() const {
        return img.width;
    }

    [[nodiscard]] Int32 getHeight() const {
        return img.height;
    }

    Color uvAt(const Vector2D& uv) const {
        return img.pixelAt(static_cast<Int32>(uv.x * img.width) % img.width, static_cast<Int32>(uv.y * img.height) % img.height);
    }

    Color uvAt(const Float u, const Float v) const {
        return img.pixelAt(static_cast<Int32>(u * img.width) % img.width, static_cast<Int32>(v * img.height) % img.height);
    }

    UInt8* getData() const {
        return img.getBuffer();
    }
};
