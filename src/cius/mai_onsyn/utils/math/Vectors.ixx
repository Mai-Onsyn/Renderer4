module;
#include <ostream>
export module Vectors;
import Types;

using std::ostream;

export struct Vector2D {
    Float x, y;

    Vector2D(Float x, Float y);

    inline Vector2D operator+(const Vector2D& other) const;
    inline Vector2D operator-(const Vector2D& v) const;
    inline Vector2D operator*(const Vector2D& v) const;
    inline Vector2D operator*(float scalar) const;
    inline Vector2D operator/(const Vector2D& v) const;
    inline Vector2D operator/(float scalar) const;
    inline Vector2D& operator+=(const Vector2D& v);
    inline Vector2D& operator-=(const Vector2D& v);
    inline Vector2D& operator*=(const Vector2D& v);
    inline Vector2D& operator*=(float scalar);
    inline Vector2D& operator/=(const Vector2D& v);
    inline Vector2D& operator/=(float scalar);
    inline Boolean operator==(const Vector2D &v) const;

    [[nodiscard]] inline Float dot(const Vector2D& v) const;
    [[nodiscard]] inline Float length() const;
    [[nodiscard]] inline Vector2D normalize() const;

    [[nodiscard]] inline String toString() const;
    friend ostream& operator<<(ostream & lhs, const Vector2D & pos);
};

export struct VectorInt2D {
    Int64 x, y;

    VectorInt2D(Int64 x, Int64 y);
    explicit operator Vector2D() const;

    inline VectorInt2D operator+(const VectorInt2D& other) const;
    inline VectorInt2D operator-(const VectorInt2D& v) const;
    inline VectorInt2D operator*(const VectorInt2D& v) const;
    inline VectorInt2D operator*(Float scalar) const;
    inline VectorInt2D operator/(const VectorInt2D& v) const;
    inline VectorInt2D operator/(Float scalar) const;
    inline VectorInt2D& operator+=(const VectorInt2D& v);
    inline VectorInt2D& operator-=(const VectorInt2D& v);
    inline VectorInt2D& operator*=(const VectorInt2D& v);
    inline VectorInt2D& operator*=(Float scalar);
    inline VectorInt2D& operator/=(const VectorInt2D& v);
    inline VectorInt2D& operator/=(Float scalar);
    inline Boolean operator==(const VectorInt2D &v) const;

    [[nodiscard]] inline Int64 dot(const VectorInt2D& v) const;
    [[nodiscard]] inline Int64 length() const;
    [[nodiscard]] inline VectorInt2D normalize() const;

    [[nodiscard]] inline String toString() const;
    friend ostream& operator<<(ostream & lhs, const VectorInt2D & pos);
};

export struct Vector3D {
    Float x, y, z;

    Vector3D(Float x, Float y, Float z);
    explicit Vector3D(const Vector2D& v2, Float z = 1.0f);
    explicit operator Vector2D() const;

    inline Vector3D operator+(const Vector3D& other) const;
    inline Vector3D operator-(const Vector3D& other) const;
    inline Vector3D operator*(const Vector3D& other) const;
    inline Vector3D operator*(Float scalar) const;
    inline Vector3D operator/(const Vector3D& other) const;
    inline Vector3D operator/(Float scalar) const;
    inline Vector3D& operator+=(const Vector3D& other);
    inline Vector3D& operator-=(const Vector3D& other);
    inline Vector3D& operator*=(const Vector3D& other);
    inline Vector3D& operator*=(Float scalar);
    inline Vector3D& operator/=(const Vector3D& other);
    inline Vector3D& operator/=(Float scalar);
    inline Boolean operator==(const Vector3D& other) const;
    [[nodiscard]] inline Float dot(const Vector3D& other) const;
    [[nodiscard]] inline Vector3D cross(const Vector3D& other) const;
    [[nodiscard]] inline Float length() const;
    [[nodiscard]] inline Vector3D normalize() const;

    [[nodiscard]] inline String toString() const;
    friend ostream& operator<<(ostream& lhs, const Vector3D& v);
};

export struct Vector4D {
    Float x, y, z, w;

    Vector4D(Float x, Float y, Float z, Float w);
    explicit Vector4D(const Vector3D& v3, Float w = 1.0f);
    explicit operator Vector3D() const;

    inline Vector4D operator+(const Vector4D& other) const;
    inline Vector4D operator-(const Vector4D& other) const;
    inline Vector4D operator*(const Vector4D& other) const;
    inline Vector4D operator*(Float scalar) const;
    inline Vector4D operator/(const Vector4D& other) const;
    inline Vector4D operator/(Float scalar) const;
    inline Vector4D& operator+=(const Vector4D& other);
    inline Vector4D& operator-=(const Vector4D& other);
    inline Vector4D& operator*=(const Vector4D& other);
    inline Vector4D& operator*=(Float scalar);
    inline Vector4D& operator/=(const Vector4D& other);
    inline Vector4D& operator/=(Float scalar);
    inline Boolean operator==(const Vector4D& other) const;

    [[nodiscard]] inline Float dot(const Vector4D& other) const;
    [[nodiscard]] inline Float length() const;
    [[nodiscard]] inline Vector4D normalize() const;

    [[nodiscard]] inline String toString() const;
    friend ostream& operator<<(ostream& lhs, const Vector4D& v);
};