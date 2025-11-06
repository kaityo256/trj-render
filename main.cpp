#include "renderer.hpp"
#include <cstdio>
#include <iostream>
#include <lammpstrj/lammpstrj.hpp>

void test_trj() {
  const char *filename = "phase_separation.lammpstrj";
  auto si = lammpstrj::read_info(filename);
  trj_render::Vector3d b1(si->x_min, si->y_min, si->z_min);
  trj_render::Vector3d b2(si->x_max, si->y_max, si->z_max);
  trj_render::Projector proj(b1, b2);
  proj.rotateY(45);
  proj.rotateZ(40);
  proj.setScale(20);
  trj_render::Renderer renderer(proj);

  lammpstrj::for_each_frame(filename,
                            [&renderer](const auto &si, auto &atoms) {
                              renderer.draw_frame(si, atoms);
                            });
}

int main() {
  test_trj();
}