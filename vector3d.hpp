#pragma once
#include <cmath>
#include <ostream>

namespace trj_render {

struct Vector2d {
  double x, y;
};

struct Vector3d {
  double x;
  double y;
  double z;

  constexpr Vector3d() : x(0.0), y(0.0), z(0.0) {}
  constexpr Vector3d(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

  constexpr Vector3d operator+(const Vector3d &rhs) const {
    return {x + rhs.x, y + rhs.y, z + rhs.z};
  }

  constexpr Vector3d operator-(const Vector3d &rhs) const {
    return {x - rhs.x, y - rhs.y, z - rhs.z};
  }

  constexpr Vector3d operator*(double s) const {
    return {x * s, y * s, z * s};
  }

  constexpr Vector3d operator/(double s) const {
    return {x / s, y / s, z / s};
  }

  Vector3d &operator+=(const Vector3d &rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  Vector3d &operator-=(const Vector3d &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  Vector3d &operator*=(double s) {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }

  Vector3d &operator/=(double s) {
    x /= s;
    y /= s;
    z /= s;
    return *this;
  }

  double norm() const {
    return std::sqrt(x * x + y * y + z * z);
  }

  double norm2() const {
    return x * x + y * y + z * z;
  }

  Vector3d normalized() const {
    double n = norm();
    return (n > 0.0) ? (*this / n) : *this;
  }

  double dot(const Vector3d &rhs) const {
    return x * rhs.x + y * rhs.y + z * rhs.z;
  }

  Vector3d cross(const Vector3d &rhs) const {
    return {
        y * rhs.z - z * rhs.y,
        z * rhs.x - x * rhs.z,
        x * rhs.y - y * rhs.x};
  }
};

inline constexpr Vector3d operator*(double s, const Vector3d &v) {
  return v * s;
}

inline std::ostream &operator<<(std::ostream &os, const Vector3d &v) {
  os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
  return os;
}

} // namespace trj_render
