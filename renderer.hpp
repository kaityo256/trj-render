#pragma once
#include "canvas.hpp"
#include "condition.hpp"
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

  std::vector<uint8_t> get_visible(Projector &proj) {
    std::vector<uint8_t> is_face_front(6, 1);
    auto v1 = proj.apply_rotation(trj_render::Vector3d(1, 0, 0));
    is_face_front[0] = (v1.x < 0);
    is_face_front[3] = !(v1.x < 0);
    auto v2 = proj.apply_rotation(trj_render::Vector3d(0, 1, 0));
    is_face_front[1] = (v2.x < 0);
    is_face_front[4] = !(v2.x < 0);
    auto v3 = proj.apply_rotation(trj_render::Vector3d(0, 0, 1));
    is_face_front[2] = (v3.x < 0);
    is_face_front[5] = !(v3.x < 0);

    std::vector<uint8_t> is_edge_visible(12, 1);

    is_edge_visible[0] = is_face_front[1] | is_face_front[2];
    is_edge_visible[1] = is_face_front[2] | is_face_front[4];
    is_edge_visible[2] = is_face_front[1] | is_face_front[5];
    is_edge_visible[3] = is_face_front[4] | is_face_front[5];
    is_edge_visible[4] = is_face_front[0] | is_face_front[2];
    is_edge_visible[5] = is_face_front[2] | is_face_front[3];
    is_edge_visible[6] = is_face_front[0] | is_face_front[5];
    is_edge_visible[7] = is_face_front[3] | is_face_front[5];
    is_edge_visible[8] = is_face_front[0] | is_face_front[1];
    is_edge_visible[9] = is_face_front[1] | is_face_front[3];
    is_edge_visible[10] = is_face_front[0] | is_face_front[4];
    is_edge_visible[11] = is_face_front[3] | is_face_front[4];

    return is_edge_visible;
  }

  void draw_simulation_box_back(Vector3d c[8], int edges[12][2], Canvas &canvas, Projector &proj) {
    auto visible = get_visible(proj);
    canvas.set_color(box_line_);
    for (int i = 0; i < 12; i++) {
      if (visible[i]) continue;
      Vector2d p1 = proj.project2d(c[edges[i][0]]);
      Vector2d p2 = proj.project2d(c[edges[i][1]]);
      canvas.moveto(p1);
      canvas.lineto(p2);
    }
  }

  void draw_simulation_box(const std::unique_ptr<lammpstrj::SystemInfo> &si, Canvas &canvas, Projector &proj, bool draw_back) {
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
    auto visible = get_visible(proj);
    canvas.set_color(box_line_);
    for (int i = 0; i < 12; i++) {
      if (visible[i] ^ draw_back) continue;
      Vector2d p1 = proj.project2d(c[edges[i][0]]);
      Vector2d p2 = proj.project2d(c[edges[i][1]]);
      canvas.moveto(p1);
      canvas.lineto(p2);
    }
  }

  void draw_simulation_box_back(const std::unique_ptr<lammpstrj::SystemInfo> &si, Canvas &canvas, Projector &proj) {
    draw_simulation_box(si, canvas, proj, false);
  }

  void draw_simulation_box_front(const std::unique_ptr<lammpstrj::SystemInfo> &si, Canvas &canvas, Projector &proj) {
    draw_simulation_box(si, canvas, proj, true);
  }

  bool check_all(lammpstrj::Atom &atom) {
    for (auto &cond : conditions_) {
      if (!cond->check(atom)) {
        return false;
      }
    }
    return true;
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
      if (!check_all(atoms[i])) continue;
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
    draw_simulation_box_back(si, canvas, projector_);
    draw_atoms(atoms, canvas, projector_);
    draw_simulation_box_front(si, canvas, projector_);
    std::ostringstream oss;
    oss << "frame." << std::setw(4) << std::setfill('0') << si->frame_index << ".png";
    std::string filename = oss.str();
    std::cout << filename << std::endl;
    canvas.save(filename.c_str());
  }
  void add_condition(std::unique_ptr<Condition> cond) {
    conditions_.push_back(std::move(cond));
  }

private:
  Projector projector_;
  Color background_;
  Color box_line_;
  std::vector<std::unique_ptr<Condition>> conditions_;
  std::array<Color, MAX_ATOM_TYPES + 1> atom_outline_;
  std::array<Color, MAX_ATOM_TYPES + 1> atom_fill_;
  std::array<double, MAX_ATOM_TYPES + 1> atom_radius_;
};
} // namespace trj_render