module;
#include <utility>
export module Texture;
import Types;
import Image;
import Color;
import Vectors;

export struct Texture {
    Color Ka;           // 环境光颜色
    Color Kd;           // 漫反射颜色
    Color Ks;           // 镜面反射颜色
    Float Ns;           // 镜面反射指数
    Float d;            // 透明度
    Image map_Kd;       // 漫反射纹理/主材质
    Image map_Ks;       // 镜面反射纹理
    Image map_d;        // 透明度纹理
    Image map_bump;     // 法线纹理

    Texture() = default;
    explicit Texture(Image img) : map_Kd(move(img)) {}
    explicit Texture(const Int32 width, const Int32 height) : map_Kd(width, height) {}
    explicit Texture(const String& path) : map_Kd(Image::fromFile(path)) {}

    explicit Texture(
        const Color& ka,
        const Color& kd,
        const Color& ks,
        const Float ns,
        const Float d
        ) : Ka(ka), Kd(kd), Ks(ks), Ns(ns), d(d) {}

    explicit Texture(
        const Color& ka,
        const Color& kd,
        const Color& ks,
        const Float ns,
        const Float d,
        const String& map_kd,
        const String& map_ks,
        const String& map_d,
        const String& map_bump
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

    explicit Texture(
        const Color& ka,
        const Color& kd,
        const Color& ks,
        const Float ns,
        const Float d,
        const Image& map_kd,
        const Image& map_ks,
        const Image& map_d,
        const Image& map_bump
        )
        : Ka(ka),
          Kd(kd),
          Ks(ks),
          Ns(ns),
          d(d),
          map_Kd(map_kd),
          map_Ks(map_ks),
          map_d(map_d),
          map_bump(map_bump) {}

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

    UInt8* getKdData() const {
        return map_Kd.getBuffer();
    }

    const Image* getKd() const {
        return &map_Kd;
    }

    const Image* getKs() const {
        return &map_Ks;
    }

    const Image* getD() const {
        return &map_d;
    }

    const Image* getBump() const {
        return &map_bump;
    }

    void setMapKd(const Image* img) {
        this->map_Kd = *img;
    }

    void setMapKs(const Image* img) {
        this->map_Ks = *img;
    }

    void setMapD(const Image* img) {
        this->map_d = *img;
    }

    void setMapBump(const Image* img) {
        this->map_bump = *img;
    }
};
