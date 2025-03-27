#ifndef VECTOR2_H
#define VECTOR2_H

#include <cmath>
#include <iostream>

class Vector2 {
public:
    float x;
    float y;

    // Constructors
    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}

    // Copy constructor
    Vector2(const Vector2& other) : x(other.x), y(other.y) {}

    // Assignment operator
    Vector2& operator=(const Vector2& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
        }
        return *this;
    }

    // Addition operators
    Vector2 operator+(const Vector2& rhs) const {
        return Vector2(x + rhs.x, y + rhs.y);
    }
    Vector2& operator+=(const Vector2& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    // Subtraction operators
    Vector2 operator-(const Vector2& rhs) const {
        return Vector2(x - rhs.x, y - rhs.y);
    }
    Vector2& operator-=(const Vector2& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    Vector2 operator-() const {
        return Vector2(-x, -y);
    }

    // Multiplication operators (scalar)
    Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }
    Vector2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    // Division operators (scalar)
    Vector2 operator/(float scalar) const {
        return Vector2(x / scalar, y / scalar);
    }
    Vector2& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Equality operators
    bool operator==(const Vector2& rhs) const {
        return x == rhs.x && y == rhs.y;
    }
    bool operator!=(const Vector2& rhs) const {
        return !(*this == rhs);
    }

    // Length of the vector
    float Length() const {
        return std::sqrt(x * x + y * y);
    }

    // Squared length (useful for performance in comparisons)
    float LengthSquared() const {
        return x * x + y * y;
    }

    // Normalize the vector in-place
    void Normalize() {
        float len = Length();
        if (len != 0) {
            x /= len;
            y /= len;
        }
    }

    // Returns a normalized copy of the vector
    Vector2 Normalized() const {
        float len = Length();
        if (len != 0) {
            return Vector2(x / len, y / len);
        }
        return Vector2(0, 0);
    }

    // Dot product
    static float Dot(const Vector2& v1, const Vector2& v2) {
        return v1.x * v2.x + v1.y * v2.y;
    }

    // Linear interpolation between two vectors
    static Vector2 Lerp(const Vector2& v1, const Vector2& v2, float t) {
        return v1 + (v2 - v1) * t;
    }

    // Distance between two vectors
    static float Distance(const Vector2& v1, const Vector2& v2) {
        return (v2 - v1).Length();
    }

    // Squared distance between two vectors
    static float DistanceSquared(const Vector2& v1, const Vector2& v2) {
        return (v2 - v1).LengthSquared();
    }

    // Reflect the vector off a surface with the given normal
    static Vector2 Reflect(const Vector2& vector, const Vector2& normal) {
        // Reflection = vector - 2 * dot(vector, normal) * normal
        return vector - normal * (2.0f * Dot(vector, normal));
    }

    // Print for debugging
    friend std::ostream& operator<<(std::ostream& os, const Vector2& vec) {
        os << "(" << vec.x << ", " << vec.y << ")";
        return os;
    }
};

// Multiplication with scalar on left-hand side
inline Vector2 operator*(float scalar, const Vector2& vec) {
    return vec * scalar;
}

#endif // VECTOR2_H
