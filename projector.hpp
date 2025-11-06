#pragma once
#include "canvas.hpp"
#include "vector3d.hpp"
#include <algorithm>
#include <array>
#include <cmath>
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
  /// @param bmin (xmin,ymin,zmin)
  /// @param bmax (xmax,ymax,zmax)
  /// @param scale 倍率（射影後の2Dに掛けるスケーリング）
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

  // 右手系、現在視点に対して後置的に回転を積む
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

  /// ワールド座標 → （重心原点へ平行移動）→ 回転 → ビュー座標
  [[nodiscard]] Vector3d to_view(const Vector3d &p_world) const {
    return R_ * Vector3d{p_world.x - center_.x,
                         p_world.y - center_.y,
                         p_world.z - center_.z};
  }

  /// 深度（手前ほど大きい）：ビュー座標の x 成分
  [[nodiscard]] double depth(const Vector3d &p_world) const {
    return to_view(p_world).x;
  }

  /// 現在の回転・倍率に対して、直方体の8頂点をすべて射影したときに完全に収まる
  /// 必要最小の (width,height) を返す（スケール込み）
  [[nodiscard]] std::pair<double, double> canvas_size() const {
    Bounds2D b = bounds2d_unscaled_();
    double w = (b.max_y - b.min_y) * scale_;
    double h = (b.max_z - b.min_z) * scale_;
    return {w, h};
  }

  /// 2D射影（スケール＆中央寄せ込み）
  /// 画像の中心 (width/2,height/2) に直方体重心が来るようオフセットする。
  [[nodiscard]] Vector2d project2d(const Vector3d &p_world) const {
    // ビュー座標（重心原点で回転済み）
    Vector3d v = to_view(p_world);

    // 射影前の2Dバウンディングを取得（回転のみ反映、スケール無し）
    Bounds2D b = bounds2d_unscaled_();

    // 2Dの中心（理論上は(0,0)だが、数値安定のために計算）
    const double cy = 0.5 * (b.min_y + b.max_y);
    const double cz = 0.5 * (b.min_z + b.max_z);

    // スケール後のキャンバスサイズ
    const double width = (b.max_y - b.min_y) * scale_;
    const double height = (b.max_z - b.min_z) * scale_;

    // 2D射影（y,z）を中心に合わせ、スケールを掛け、画像中心へ平行移動
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

  void draw_simulation_box(trj_render::canvas &canvas) {
    trj_render::Vector3d c[8] = {
        {bmin_.x, bmin_.y, bmin_.z}, // 0
        {bmax_.x, bmin_.y, bmin_.z}, // 1
        {bmin_.x, bmax_.y, bmin_.z}, // 2
        {bmax_.x, bmax_.y, bmin_.z}, // 3
        {bmin_.x, bmin_.y, bmax_.z}, // 4
        {bmax_.x, bmin_.y, bmax_.z}, // 5
        {bmin_.x, bmax_.y, bmax_.z}, // 6
        {bmax_.x, bmax_.y, bmax_.z}  // 7
    };

    // 各辺を構成する頂点インデックスのペア (12本)
    int edges[12][2] = {
        {0, 1}, {2, 3}, {4, 5}, {6, 7}, // x方向の4本
        {0, 2},
        {1, 3},
        {4, 6},
        {5, 7}, // y方向の4本
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7} // z方向の4本
    };

    // 各辺を描画
    canvas.set_color(0, 0, 0);
    for (auto &e : edges) {
      trj_render::Vector2d p1 = project2d(c[e[0]]);
      trj_render::Vector2d p2 = project2d(c[e[1]]);
      canvas.moveto(p1);
      canvas.lineto(p2);
    }
  }

private:
  struct Bounds2D {
    double min_y, max_y; // ビュー座標の y 範囲
    double min_z, max_z; // ビュー座標の z 範囲
  };

  Vector3d bmin_, bmax_;
  Vector3d center_;
  double scale_;
  Mat3d R_;

  // 直方体の8頂点（ワールド）を列挙
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

  // 現在の回転 R_ に基づき、（重心原点にして）2D(y,z)の未スケール境界を得る
  Bounds2D bounds2d_unscaled_() const {
    auto cs = corners_();
    double miny = +1e300, maxy = -1e300;
    double minz = +1e300, maxz = -1e300;
    for (const auto &p : cs) {
      Vector3d v = to_view(p); // center 平行移動 → 回転
      miny = std::min(miny, v.y);
      maxy = std::max(maxy, v.y);
      minz = std::min(minz, v.z);
      maxz = std::max(maxz, v.z);
    }
    return {miny, maxy, minz, maxz};
  }

  // 回転行列ユーティリティ
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
