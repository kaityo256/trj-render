#pragma once
#include "canvas.hpp"
#include "vector3d.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <lammpstrj/lammpstrj.hpp>
#include <utility>

namespace trj_render {

struct Mat3d {
  double m[3][3];
  static Mat3d identity() {
    Mat3d I{};
    I.m[0][0] = 1;
    I.m[0][1] = 0;
    I.m[0][2] = 0;
    I.m[1][0] = 0;
    I.m[1][1] = 1;
    I.m[1][2] = 0;
    I.m[2][0] = 0;
    I.m[2][1] = 0;
    I.m[2][2] = 1;
    return I;
  }
  friend Mat3d operator*(const Mat3d &A, const Mat3d &B) {
    Mat3d C{};
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        C.m[i][j] = A.m[i][0] * B.m[0][j] + A.m[i][1] * B.m[1][j] + A.m[i][2] * B.m[2][j];
    return C;
  }
  friend Vector3d operator*(const Mat3d &A, const Vector3d &v) {
    return {
        A.m[0][0] * v.x + A.m[0][1] * v.y + A.m[0][2] * v.z,
        A.m[1][0] * v.x + A.m[1][1] * v.y + A.m[1][2] * v.z,
        A.m[2][0] * v.x + A.m[2][1] * v.y + A.m[2][2] * v.z};
  }
};

class Projector {
public:
  Projector(const Vector3d &bmin, const Vector3d &bmax, double scale = 1.0)
      : bmin_(bmin), bmax_(bmax), scale_(scale), R_(Mat3d::identity()) {
    center_.x = 0.5 * (bmin_.x + bmax_.x);
    center_.y = 0.5 * (bmin_.y + bmax_.y);
    center_.z = 0.5 * (bmin_.z + bmax_.z);
  }

  void resetRotation() {
    R_ = Mat3d::identity();
  }

  void setScale(double s) {
    scale_ = s;
  }
  double scale() const {
    return scale_;
  }

  void rotateX(double a) {
    a = a / 180 * M_PI;
    R_ = R_ * rotX(a);
  }
  void rotateY(double a) {
    a = a / 180 * M_PI;
    R_ = R_ * rotY(a);
  }
  void rotateZ(double a) {
    a = a / 180 * M_PI;
    R_ = R_ * rotZ(a);
  }

  [[nodiscard]] Vector3d to_view(const Vector3d &p_world) const {
    return R_ * Vector3d{p_world.x - center_.x,
                         p_world.y - center_.y,
                         p_world.z - center_.z};
  }

  [[nodiscard]] double depth(const Vector3d &p_world) const {
    return to_view(p_world).x;
  }

  [[nodiscard]] std::pair<double, double> canvas_size() const {
    Bounds2D b = bounds2d_unscaled_();
    double w = (b.max_y - b.min_y) * scale_;
    double h = (b.max_z - b.min_z) * scale_;
    return {w, h};
  }

  [[nodiscard]] Vector2d project2d(const Vector3d &p_world) const {
    Vector3d v = to_view(p_world);
    Bounds2D b = bounds2d_unscaled_();
    const double cy = 0.5 * (b.min_y + b.max_y);
    const double cz = 0.5 * (b.min_z + b.max_z);
    const double width = (b.max_y - b.min_y) * scale_;
    const double height = (b.max_z - b.min_z) * scale_;
    double sx = (v.y - cy) * scale_ + 0.5 * width;
    double sy = (v.z - cz) * scale_ + 0.5 * height;
    return {sx, sy};
  }

  void test() {
    Vector3d s1 = Vector3d(bmin_.x, bmin_.y, bmin_.z);
    Vector3d s2 = Vector3d(bmin_.x, bmin_.y, bmin_.z);
    Vector3d s3 = Vector3d(bmin_.x, bmin_.y, bmin_.z);
    Vector3d s4 = Vector3d(bmin_.x, bmin_.y, bmin_.z);
    Vector3d s5 = Vector3d(bmin_.x, bmin_.y, bmin_.z);
    Vector3d s6 = Vector3d(bmin_.x, bmin_.y, bmin_.z);
    Vector3d s7 = Vector3d(bmin_.x, bmin_.y, bmin_.z);
    Vector3d s8 = Vector3d(bmin_.x, bmin_.y, bmin_.z);
  }

private:
  struct Bounds2D {
    double min_y, max_y;
    double min_z, max_z;
  };

  Vector3d bmin_, bmax_;
  Vector3d center_;
  double scale_;
  Mat3d R_;

  std::array<Vector3d, 8> corners_() const {
    const double xs[2] = {bmin_.x, bmax_.x};
    const double ys[2] = {bmin_.y, bmax_.y};
    const double zs[2] = {bmin_.z, bmax_.z};
    std::array<Vector3d, 8> c{};
    int k = 0;
    for (int ix = 0; ix < 2; ++ix)
      for (int iy = 0; iy < 2; ++iy)
        for (int iz = 0; iz < 2; ++iz)
          c[k++] = {xs[ix], ys[iy], zs[iz]};
    return c;
  }

  Bounds2D bounds2d_unscaled_() const {
    auto cs = corners_();
    double miny = +1e300, maxy = -1e300;
    double minz = +1e300, maxz = -1e300;
    for (const auto &p : cs) {
      Vector3d v = to_view(p);
      miny = std::min(miny, v.y);
      maxy = std::max(maxy, v.y);
      minz = std::min(minz, v.z);
      maxz = std::max(maxz, v.z);
    }
    return {miny, maxy, minz, maxz};
  }

  static Mat3d rotX(double a) {
    const double c = std::cos(a), s = std::sin(a);
    Mat3d M{};
    M.m[0][0] = 1;
    M.m[0][1] = 0;
    M.m[0][2] = 0;
    M.m[1][0] = 0;
    M.m[1][1] = c;
    M.m[1][2] = -s;
    M.m[2][0] = 0;
    M.m[2][1] = s;
    M.m[2][2] = c;
    return M;
  }
  static Mat3d rotY(double a) {
    const double c = std::cos(a), s = std::sin(a);
    Mat3d M{};
    M.m[0][0] = c;
    M.m[0][1] = 0;
    M.m[0][2] = s;
    M.m[1][0] = 0;
    M.m[1][1] = 1;
    M.m[1][2] = 0;
    M.m[2][0] = -s;
    M.m[2][1] = 0;
    M.m[2][2] = c;
    return M;
  }
  static Mat3d rotZ(double a) {
    const double c = std::cos(a), s = std::sin(a);
    Mat3d M{};
    M.m[0][0] = c;
    M.m[0][1] = -s;
    M.m[0][2] = 0;
    M.m[1][0] = s;
    M.m[1][1] = c;
    M.m[1][2] = 0;
    M.m[2][0] = 0;
    M.m[2][1] = 0;
    M.m[2][2] = 1;
    return M;
  }
};

} // namespace trj_render
