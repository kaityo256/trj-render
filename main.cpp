#include "renderer.hpp"
#include <cstdio>
#include <iostream>
#include <lammpstrj/lammpstrj.hpp>

void test_trj() {
  const char *filename = "collision.lammpstrj";
  trj_render::Renderer renderer;
  lammpstrj::for_each_frame(
      filename,
      // renderer を参照キャプチャ
      [&renderer](const std::unique_ptr<lammpstrj::SystemInfo> &si,
                  std::vector<lammpstrj::Atom> &atoms) {
        renderer.draw_frame(si, atoms);
      });
}

int main() {
  test_trj();
}