module;
#include <immintrin.h>
export module SIMD;
import Types;

export class Vec8i;

export class Vec8f {
    __m256 val;

public:
    inline Vec8f(__m256 v) : val(v) {
    }

    inline Vec8f(Float v) : val(_mm256_set1_ps(v)) {
    }

    inline Vec8f(const Float v1, const Float v2, const Float v3, const Float v4, const Float v5, const Float v6,
                 const Float v7, const Float v8) : val(_mm256_set_ps(v8, v7, v6, v5, v4, v3, v2, v1)) {
    }

    inline operator Vec8i() const;

    inline Vec8f operator+(const Vec8f& o) const { return _mm256_add_ps(val, o.val); }
    inline Vec8f operator-(const Vec8f& o) const { return _mm256_sub_ps(val, o.val); }
    inline Vec8f operator*(const Vec8f& o) const { return _mm256_mul_ps(val, o.val); }
    inline Vec8f operator/(const Vec8f& o) const { return _mm256_div_ps(val, o.val); }

    inline void store(Float* ptr) const { _mm256_store_ps(ptr, val); }

    inline static Vec8f andNot(const Vec8f& a, const Vec8f& b) { return _mm256_andnot_ps(a.val, b.val); }
    inline static Vec8f max(const Vec8f& a, const Vec8f& b) { return _mm256_max_ps(a.val, b.val); }
    inline static Vec8f min(const Vec8f& a, const Vec8f& b) { return _mm256_min_ps(a.val, b.val); }
};

export class Vec8i {
    __m256i val;

public:
    inline Vec8i(__m256i v) : val(v) {}
    inline Vec8i(const Int32 v) : val(_mm256_set1_epi32(v)) {}
    inline Vec8i(const Int32 v1, const Int32 v2, const Int32 v3, const Int32 v4, const Int32 v5, const Int32 v6, const Int32 v7, const Int32 v8) :
        val(_mm256_set_epi32(v8, v7, v6, v5, v4, v3, v2, v1)) {}

    inline operator Vec8f() const;

    inline Vec8i operator+(const Vec8i& o) const { return Vec8i(_mm256_add_epi32(val, o.val)); }
    inline Vec8i operator-(const Vec8i& o) const { return Vec8i(_mm256_sub_epi32(val, o.val)); }
    inline Vec8i operator*(const Vec8i& o) const { return Vec8i(_mm256_mullo_epi32(val, o.val)); }

    inline void store(Int32* buffer) const { _mm256_store_si256(reinterpret_cast<__m256i *>(buffer), val); }

    inline Vec8i clamp(const Int32 lo, const Int32 hi) const {
        return _mm256_min_epi32(_mm256_max_epi32(val, _mm256_set1_epi32(lo)), _mm256_set1_epi32(hi));
    }

    inline Vec8i clamp(const Vec8i& lo, const Vec8i& hi) const {
        return _mm256_min_epi32(_mm256_max_epi32(val, lo.val), hi.val);
    }

    inline static Vec8i max(const Vec8i& a, const Vec8i& b) { return Vec8i(_mm256_min_epi32(a.val, b.val)); }
    inline static Vec8i min(const Vec8i& a, const Vec8i& b) { return Vec8i(_mm256_max_epi32(a.val, b.val)); }
};

inline Vec8f::operator Vec8i() const { return {_mm256_cvttps_epi32(val)}; }

inline Vec8i::operator Vec8f() const { return {_mm256_cvtepi32_ps(val)}; }