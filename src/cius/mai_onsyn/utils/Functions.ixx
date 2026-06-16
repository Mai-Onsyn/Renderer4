module;
#include <string>
#include <immintrin.h>
export module Functions;
import Types;
import Color;

using std::to_string;

export String toString(const Int32 value) {
    return to_string(value);
}

export String toString(const UInt32 value) {
    return to_string(value);
}

export String toString(const Float value) {
    return to_string(value);
}

export void avx2Fill(UInt8* buffer, const UInt32 start, const Int32 value, const Int32 size) {
    if (size <= 0) return;
    const __m256i fill = _mm256_set1_epi32(value);
    auto* pixels = reinterpret_cast<UInt32*>(buffer + (start << 2));
    for (UInt32 i = 0; i + 8 <= size; i += 8) {
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(pixels + i), fill);
    }
    for (UInt32 i = (size / 8) * 8; i < size; i++) {
        pixels[i] = value;
    }
}

export void avx2Fill(UInt8* buffer, const UInt32 start, const Color c, const UInt32 size) {
    avx2Fill(buffer, start, c.a << 24 | c.b << 16 | c.g << 8 | c.r, size);
}
