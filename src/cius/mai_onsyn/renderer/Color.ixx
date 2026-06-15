module;
#include <string>
export module Color;
import Types;
import Format;

export struct Color {
    UInt8 r, g, b, a = 255;

    constexpr Color(const UInt8 r, const UInt8 g, const UInt8 b, const UInt8 a = 255) noexcept
        : r(r), g(g), b(b), a(a) {}

    constexpr Color() = default;

    String toString() const {
        return format("Color{%d, %d, %d, %d}", r, g, b, a);
    }

    static const Color Transparent;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Magenta;
    static const Color Cyan;
    static const Color White;
    static const Color Black;

    static const Color SkyBlue;
};

export constexpr Color Color::Transparent{255, 255, 255, 0};
export constexpr Color Color::Red{255, 0, 0};
export constexpr Color Color::Green{0, 255, 0};
export constexpr Color Color::Blue{0, 0, 255};
export constexpr Color Color::Yellow{255, 255, 0};
export constexpr Color Color::Magenta{255, 0, 255};
export constexpr Color Color::Cyan{0, 255, 255};
export constexpr Color Color::White{255, 255, 255};
export constexpr Color Color::Black{0, 0, 0};

export constexpr Color Color::SkyBlue{135, 206, 250};