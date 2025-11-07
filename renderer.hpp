#pragma once
#include "canvas.hpp"
#include "projector.hpp"
#include "vector3d.hpp"
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <lammpstrj/lammpstrj.hpp>
namespace trj_render {

inline constexpr int MAX_ATOM_TYPES = 16;

class Renderer {
public:
  Renderer(Projector &projector) : projector_(projector) {
    background_ = {0, 0, 0};
    box_line_ = {255, 255, 255};

    for (int t = 0; t <= MAX_ATOM_TYPES; ++t) {
      atom_outline_[t] = {0, 0, 0};
      atom_fill_[t] = {64, 128, 255};
      atom_radius_[t] = 0.5;
    }
    if (MAX_ATOM_TYPES >= 1) atom_fill_[1] = {230, 64, 64};  // 赤
    if (MAX_ATOM_TYPES >= 2) atom_fill_[2] = {64, 200, 64};  // 緑
    if (MAX_ATOM_TYPES >= 3) atom_fill_[3] = {64, 100, 255}; // 青
    if (MAX_ATOM_TYPES >= 4) atom_fill_[4] = {255, 210, 64}; // 黄
  }

  void set_atom_radius(int type, double radius) {
    atom_radius_[type] = radius;
  }

  void draw_simulation_box(const std::unique_ptr<lammpstrj::SystemInfo> &si, Canvas &canvas, Projector &proj) {
    Vector3d c[8] = {
        {si->x_min, si->y_min, si->z_min}, // 0
        {si->x_max, si->y_min, si->z_min}, // 1
        {si->x_min, si->y_max, si->z_min}, // 2
        {si->x_max, si->y_max, si->z_min}, // 3
        {si->x_min, si->y_min, si->z_max}, // 4
        {si->x_max, si->y_min, si->z_max}, // 5
        {si->x_min, si->y_max, si->z_max}, // 6
        {si->x_max, si->y_max, si->z_max}  // 7
    };
    int edges[12][2] = {
        {0, 1}, {2, 3}, {4, 5}, {6, 7}, {0, 2}, {1, 3}, {4, 6}, {5, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

    canvas.set_color(box_line_);
    for (auto &e : edges) {
      Vector2d p1 = proj.project2d(c[e[0]]);
      Vector2d p2 = proj.project2d(c[e[1]]);
      canvas.moveto(p1);
      canvas.lineto(p2);
    }
  }

  void draw_atoms(std::vector<lammpstrj::Atom> &atoms, Canvas &canvas, Projector &proj) {
    std::vector<Vector3d> pos;
    for (auto a : atoms) {
      pos.push_back(Vector3d(a.x, a.y, a.z));
    }
    std::vector<std::size_t> idx(atoms.size());
    for (std::size_t i = 0; i < atoms.size(); ++i) {
      idx[i] = i;
    }
    std::sort(idx.begin(), idx.end(),
              [&](std::size_t ia, std::size_t ib) {
                double da = proj.depth(pos[ia]);
                double db = proj.depth(pos[ib]);
                return da < db;
              });
    for (std::size_t i : idx) {
      const auto t = atoms[i].type;
      const double r = atom_radius_[t] * proj.scale();
      Vector2d s = proj.project2d(pos[i]);
      canvas.set_color(atom_fill_[t]);
      canvas.fill_circle(s.x, s.y, r);
      canvas.set_color(atom_outline_[t]);
      canvas.draw_circle(s.x, s.y, r);
    }
  }

  void draw_frame(const std::unique_ptr<lammpstrj::SystemInfo> &si,
                  std::vector<lammpstrj::Atom> &atoms) {
    auto [width, height] = projector_.canvas_size();
    Canvas canvas(width, height);
    canvas.set_color(background_);
    canvas.fill_rect(0, 0, width, height);
    draw_simulation_box(si, canvas, projector_);
    draw_atoms(atoms, canvas, projector_);
    std::ostringstream oss;
    oss << "frame." << std::setw(4) << std::setfill('0') << si->frame_index << ".png";
    std::string filename = oss.str();
    std::cout << filename << std::endl;
    canvas.save(filename.c_str());
  }

private:
  Projector projector_;
  Color background_;
  Color box_line_;
  std::array<Color, MAX_ATOM_TYPES + 1> atom_outline_;
  std::array<Color, MAX_ATOM_TYPES + 1> atom_fill_;
  std::array<double, MAX_ATOM_TYPES + 1> atom_radius_;
};
} // namespace trj_render