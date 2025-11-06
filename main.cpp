#include "canvas.hpp"
#include "projector.hpp"
#include "vector3d.hpp"
#include <cstdio>
#include <iostream>
#include <lammpstrj/lammpstrj.hpp>

void draw_simulation_box(const std::unique_ptr<lammpstrj::SystemInfo> &si, trj_render::Canvas &canvas, trj_render::Projector &proj) {
  trj_render::Vector3d c[8] = {
      {si->x_min, si->y_min, si->z_min}, // 0
      {si->x_max, si->y_min, si->z_min}, // 1
      {si->x_min, si->y_max, si->z_min}, // 2
      {si->x_max, si->y_max, si->z_min}, // 3
      {si->x_min, si->y_min, si->z_max}, // 4
      {si->x_max, si->y_min, si->z_max}, // 5
      {si->x_min, si->y_max, si->z_max}, // 6
      {si->x_max, si->y_max, si->z_max}  // 7
  };

  // 各辺を構成する頂点インデックスのペア (12本)
  int edges[12][2] = {
      {0, 1}, {2, 3}, {4, 5}, {6, 7}, {0, 2}, {1, 3}, {4, 6}, {5, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

  // 各辺を描画
  canvas.set_color(0, 0, 0);
  for (auto &e : edges) {
    trj_render::Vector2d p1 = proj.project2d(c[e[0]]);
    trj_render::Vector2d p2 = proj.project2d(c[e[1]]);
    canvas.moveto(p1);
    canvas.lineto(p2);
  }
}

void draw_atoms(std::vector<lammpstrj::Atom> &atoms, trj_render::Canvas &canvas, trj_render::Projector &proj) {
  std::vector<trj_render::Vector3d> pos;
  for (auto a : atoms) {
    pos.push_back(trj_render::Vector3d(a.x, a.y, a.z));
  }
  std::vector<std::size_t> idx(atoms.size());
  for (std::size_t i = 0; i < atoms.size(); ++i) {
    idx[i] = i;
  }
  canvas.set_color(255, 0, 0);
  std::sort(idx.begin(), idx.end(),
            [&](std::size_t ia, std::size_t ib) {
              double da = proj.depth(pos[ia]);
              double db = proj.depth(pos[ib]);
              return da < db;
            });
  for (std::size_t i : idx) {
    trj_render::Vector2d s = proj.project2d(pos[i]);
    canvas.set_color(255, 0, 0);
    canvas.fill_circle(s.x, s.y, 5);
    canvas.set_color(0, 0, 0);
    canvas.draw_circle(s.x, s.y, 5);
  }
}

void draw_frame(const std::unique_ptr<lammpstrj::SystemInfo> &si,
                std::vector<lammpstrj::Atom> &atoms) {
  static int index = 0;
  trj_render::Vector3d b1(si->x_min, si->y_min, si->z_min);
  trj_render::Vector3d b2(si->x_max, si->y_max, si->z_max);
  trj_render::Projector proj(b1, b2);
  proj.rotateY(45);
  proj.rotateZ(30);
  proj.setScale(10);
  auto [width, height] = proj.canvas_size();
  trj_render::Canvas canvas(width, height);
  draw_simulation_box(si, canvas, proj);
  draw_atoms(atoms, canvas, proj);
  std::ostringstream oss;
  oss << "frame." << std::setw(4) << std::setfill('0') << index << ".png";
  std::string filename = oss.str();
  std::cout << filename << std::endl;
  canvas.save(filename.c_str());
  index++;
}

void test_trj() {
  const char *filename = "collision.lammpstrj";
  lammpstrj::for_each_frame(filename, [](const std::unique_ptr<lammpstrj::SystemInfo> &si, std::vector<lammpstrj::Atom> &atoms) { draw_frame(si, atoms); });
}

int main() {
  test_trj();
}