module;
#include <immintrin.h>
export module SIMD;
import Types;

export class Vec8f {
    __m256 val;
public:
    inline Vec8f(__m256 v) : val(v) {}
    inline Vec8f(const Float v) : val(_mm256_set1_ps(v)) {}
    inline Vec8f(const Float v1, const Float v2, const Float v3, const Float v4, const Float v5, const Float v6, const Float v7, const Float v8) : val(_mm256_set_ps(v8, v7, v6, v5, v4, v3, v2, v1)) {}

    inline Vec8f operator+(const Vec8f& o) const { return _mm256_add_ps(val, o.val); }

    inline Vec8f operator-(const Vec8f& o) const { return _mm256_sub_ps(val, o.val); }

    inline Vec8f operator*(const Vec8f& o) const { return _mm256_mul_ps(val, o.val); }

    inline Vec8f operator/(const Vec8f& o) const { return _mm256_div_ps(val, o.val); }

    inline void store(Float* ptr) const { _mm256_store_ps(ptr, val); }

    inline static Vec8f andNot(const Vec8f& a, const Vec8f& b) { return _mm256_andnot_ps(a.val, b.val); }
};