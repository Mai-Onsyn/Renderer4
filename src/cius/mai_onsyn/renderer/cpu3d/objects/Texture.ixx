module;
#include <utility>
export module Texture;
import Types;
import Image;
import Color;
import Vectors;

export struct Texture {
private:
    Color Ka;           // 环境光颜色
    Color Kd;           // 漫反射颜色
    Color Ks;           // 镜面反射颜色
    Float Ns;           // 镜面反射指数
    Float d;            // 透明度
    Image map_Kd;       // 漫反射纹理/主材质
    Image map_Ks;       // 镜面反射纹理
    Image map_d;        // 透明度纹理
    Image map_bump;     // 法线纹理
public:
    explicit Texture(Image img) : map_Kd(move(img)) {}
    explicit Texture(const Int32 width, const Int32 height) : map_Kd(width, height) {}
    explicit Texture(const String& path) : map_Kd(Image::fromFile(path)) {}
    Texture(
        const Color& ka = Color::White,
        const Color& kd = Color::White,
        const Color& ks = Color::White,
        const Float ns = 0,
        const Float d = 1,
        const String& map_kd = "",
        const String& map_ks = "",
        const String& map_d = "",
        const String& map_bump = ""
        )
        : Ka(ka),
          Kd(kd),
          Ks(ks),
          Ns(ns),
          d(d),
          map_Kd(Image::fromFile(map_kd)),
          map_Ks(Image::fromFile(map_ks)),
          map_d(Image::fromFile(map_d)),
          map_bump(Image::fromFile(map_bump)) {}

    [[nodiscard]] Int32 getWidth() const {
        return map_Kd.width;
    }

    [[nodiscard]] Int32 getHeight() const {
        return map_Kd.height;
    }

    Color uvAt(const Vector2D& uv) const {
        return map_Kd.pixelAt(static_cast<Int32>(uv.x * map_Kd.width) % map_Kd.width, static_cast<Int32>(uv.y * map_Kd.height) % map_Kd.height);
    }

    Color uvAt(const Float u, const Float v) const {
        return map_Kd.pixelAt(static_cast<Int32>(u * map_Kd.width) % map_Kd.width, static_cast<Int32>(v * map_Kd.height) % map_Kd.height);
    }

    UInt8* getData() const {
        return map_Kd.getBuffer();
    }
};
