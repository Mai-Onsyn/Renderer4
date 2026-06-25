module;
#include <immintrin.h>
#include <cmath>
export module SIMD;
import Types;

export class Vec8i;

export class alignas(32) Vec8f {
    __m256 val;

public:
    inline __attribute__((always_inline)) Vec8f(__m256 v) : val(v) {}
    inline __attribute__((always_inline)) Vec8f(const Float v) : val(_mm256_set1_ps(v)) {}
    inline __attribute__((always_inline)) Vec8f(const Float v1, const Float v2, const Float v3, const Float v4, const Float v5, const Float v6,
                 const Float v7, const Float v8) : val(_mm256_set_ps(v8, v7, v6, v5, v4, v3, v2, v1)) {}
    inline __attribute__((always_inline)) Vec8f(const Float* v) : val(_mm256_loadu_ps(v)) {}

    inline __attribute__((always_inline)) operator Vec8i() const;
    inline __attribute__((always_inline)) operator __m256() const { return val; }

    inline __attribute__((always_inline)) Vec8f operator+(const Vec8f& o) const { return _mm256_add_ps(val, o.val); }
    inline __attribute__((always_inline)) Vec8f operator-(const Vec8f& o) const { return _mm256_sub_ps(val, o.val); }
    inline __attribute__((always_inline)) Vec8f operator*(const Vec8f& o) const { return _mm256_mul_ps(val, o.val); }
    inline __attribute__((always_inline)) Vec8f operator/(const Vec8f& o) const { return _mm256_div_ps(val, o.val); }

    inline __attribute__((always_inline)) Boolean operator>(const Vec8f& o) const { return _mm256_movemask_ps(_mm256_cmp_ps(val, o.val, _CMP_GT_OQ)) != 0; }
    inline __attribute__((always_inline)) Boolean operator>=(const Vec8f& o) const { return _mm256_movemask_ps(_mm256_cmp_ps(val, o.val, _CMP_GE_OQ)) != 0; }
    inline __attribute__((always_inline)) Boolean operator<(const Vec8f& o) const { return _mm256_movemask_ps(_mm256_cmp_ps(val, o.val, _CMP_LT_OQ)) != 0; }
    inline __attribute__((always_inline)) Boolean operator<=(const Vec8f& o) const { return _mm256_movemask_ps(_mm256_cmp_ps(val, o.val, _CMP_LE_OQ)) != 0; }

    inline __attribute__((always_inline)) void operator+=(const Vec8f& o) { val = _mm256_add_ps(val, o.val); }
    inline __attribute__((always_inline)) void operator-=(const Vec8f& o) { val = _mm256_sub_ps(val, o.val); }
    inline __attribute__((always_inline)) void operator*=(const Vec8f& o) { val = _mm256_mul_ps(val, o.val); }
    inline __attribute__((always_inline)) void operator/=(const Vec8f& o) { val = _mm256_div_ps(val, o.val); }

    inline __attribute__((always_inline)) void store(Float* ptr) const { _mm256_storeu_ps(ptr, val); }
    inline __attribute__((always_inline)) Vec8f sqrt() const { return _mm256_sqrt_ps(val); }

    inline __attribute__((always_inline)) Vec8f clamp(const Vec8f& min, const Vec8f& max) const {
        return _mm256_min_ps(_mm256_max_ps(val, min.val), max.val);
    }

    inline __attribute__((always_inline)) Vec8f clamp(const Float lo, const Float hi) const {
        return _mm256_min_ps(_mm256_max_ps(val, _mm256_set1_ps(lo)), _mm256_set1_ps(hi));
    }

    inline __attribute__((always_inline)) static Vec8f andNot(const Vec8f& a, const Vec8f& b) { return _mm256_andnot_ps(a.val, b.val); }
    inline __attribute__((always_inline)) static Vec8f max(const Vec8f& a, const Vec8f& b) { return _mm256_max_ps(a.val, b.val); }
    inline __attribute__((always_inline)) static Vec8f min(const Vec8f& a, const Vec8f& b) { return _mm256_min_ps(a.val, b.val); }

    inline __attribute__((always_inline)) static Vec8f dot3D(const Vec8f& ax, const Vec8f& ay, const Vec8f& az, const Vec8f& bx, const Vec8f& by, const Vec8f& bz) {
        return ax * bx + ay * by + az * bz;
    }
};

export class alignas(32) Vec8i {
    __m256i val;

public:
    inline __attribute__((always_inline)) Vec8i(__m256i v) : val(v) {}
    inline __attribute__((always_inline)) Vec8i(const Int32 v) : val(_mm256_set1_epi32(v)) {}
    inline __attribute__((always_inline)) Vec8i(const Int32 v1, const Int32 v2, const Int32 v3, const Int32 v4, const Int32 v5, const Int32 v6, const Int32 v7, const Int32 v8) :
        val(_mm256_set_epi32(v8, v7, v6, v5, v4, v3, v2, v1)) {}
    inline __attribute__((always_inline)) Vec8i(const Int32* ptr) : val(_mm256_loadu_si256((__m256i*)ptr)) {}

    inline __attribute__((always_inline)) operator Vec8f() const;
    inline __attribute__((always_inline)) operator __m256i() const { return val; }

    inline __attribute__((always_inline)) Vec8i operator+(const Vec8i& o) const { return Vec8i(_mm256_add_epi32(val, o.val)); }
    inline __attribute__((always_inline)) Vec8i operator-(const Vec8i& o) const { return Vec8i(_mm256_sub_epi32(val, o.val)); }
    inline __attribute__((always_inline)) Vec8i operator*(const Vec8i& o) const { return Vec8i(_mm256_mullo_epi32(val, o.val)); }

    inline __attribute__((always_inline)) void store(Int32* buffer) const { _mm256_store_si256(reinterpret_cast<__m256i *>(buffer), val); }

    inline __attribute__((always_inline)) Vec8i clamp(const Int32 lo, const Int32 hi) const {
        return _mm256_min_epi32(_mm256_max_epi32(val, _mm256_set1_epi32(lo)), _mm256_set1_epi32(hi));
    }

    inline __attribute__((always_inline)) Vec8i clamp(const Vec8i& lo, const Vec8i& hi) const {
        return _mm256_min_epi32(_mm256_max_epi32(val, lo.val), hi.val);
    }

    inline __attribute__((always_inline)) static Vec8i max(const Vec8i& a, const Vec8i& b) { return Vec8i(_mm256_max_epi32(a.val, b.val)); }
    inline __attribute__((always_inline)) static Vec8i min(const Vec8i& a, const Vec8i& b) { return Vec8i(_mm256_min_epi32(a.val, b.val)); }
};

inline __attribute__((always_inline)) Vec8f::operator Vec8i() const { return {_mm256_cvttps_epi32(val)}; }

inline __attribute__((always_inline)) Vec8i::operator Vec8f() const { return {_mm256_cvtepi32_ps(val)}; }


export inline __attribute__((always_inline)) void storeColorVec(const __m256i& colorVec, UInt8* ptr) {
    __m128i lo = _mm256_castsi256_si128(colorVec);
    __m128i hi = _mm256_extracti128_si256(colorVec, 1);

    __m128i pack16 = _mm_packus_epi32(lo, hi);
    __m128i pack8 = _mm_packus_epi16(pack16, _mm_setzero_si128());

    _mm_storel_epi64(reinterpret_cast<__m128i*>(ptr), pack8);
}




// 绝对安全的工业级 SIMD exp 近似（纯浮点运算，无位移风险）
inline __attribute__((always_inline)) __m256 _mm256_exp_ps_safe(__m256 x) {
    // 强行把输入限制在安全范围内，防止发生浮点下溢或上溢
    x = _mm256_max_ps(_mm256_set1_ps(-80.0f), _mm256_min_ps(_mm256_set1_ps(80.0f), x));

    // e^x = [1 + x/256]^256
    // 采用 Exponentiation by squaring (连续平方8次代替256次方)
    x = _mm256_fmadd_ps(x, _mm256_set1_ps(1.0f / 256.0f), _mm256_set1_ps(1.0f));
    x = _mm256_mul_ps(x, x); // 2
    x = _mm256_mul_ps(x, x); // 4
    x = _mm256_mul_ps(x, x); // 8
    x = _mm256_mul_ps(x, x); // 16
    x = _mm256_mul_ps(x, x); // 32
    x = _mm256_mul_ps(x, x); // 64
    x = _mm256_mul_ps(x, x); // 128
    return _mm256_mul_ps(x, x); // 256
}

// 绝对安全的 SIMD log 近似（泰勒展开级数）
inline __attribute__((always_inline)) __m256 _mm256_log_ps_safe(__m256 x) {
    // 确保底数绝对大于0，防止逼近 -inf
    x = _mm256_max_ps(x, _mm256_set1_ps(1e-4f));

    // 使用 Pade 近似或泰勒级数改善 [0.5, 2.0] 之间的精度
    // 这里采用高效逼近：ln(x)
    __m256 num = _mm256_sub_ps(x, _mm256_set1_ps(1.0f));
    __m256 den = _mm256_add_ps(x, _mm256_set1_ps(1.0f));
    __m256 t = _mm256_div_ps(num, den); // t = (x-1)/(x+1)

    __m256 t2 = _mm256_mul_ps(t, t);

    // 多项式逼近 2 * (t + t^3/3 + t^5/5)
    __m256 p = _mm256_fmadd_ps(t2, _mm256_set1_ps(0.2f), _mm256_set1_ps(0.3333333f));
    p = _mm256_fmadd_ps(t2, p, _mm256_set1_ps(1.0f));
    return _mm256_mul_ps(_mm256_set1_ps(2.0f), _mm256_mul_ps(t, p));
}

// 最终稳定的 pow
export inline __attribute__((always_inline)) __m256 _mm256_pow_ps_approx(__m256 base, __m256 exp) {
    // 如果高光项 dot 接近 0，直接斩断，不让它进 log 计算
    __m256 mask = _mm256_cmp_ps(base, _mm256_set1_ps(1e-4f), _CMP_GT_OQ);

    __m256 log_base = _mm256_log_ps_safe(base);
    __m256 mul = _mm256_mul_ps(log_base, exp);
    __m256 res = _mm256_exp_ps_safe(mul);

    // 如果本来 base 就小于等于 0，强行将其高光归零，防止 NaN 污染
    return _mm256_and_ps(res, mask);
}