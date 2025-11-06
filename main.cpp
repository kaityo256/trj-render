#include "draw.hpp"
#include <cstdio>
#include <iostream>
#include <lammpstrj/lammpstrj.hpp>

void test_trj() {
  const char *filename = "collision.lammpstrj";
  lammpstrj::for_each_frame(filename, [](const std::unique_ptr<lammpstrj::SystemInfo> &si, std::vector<lammpstrj::Atom> &atoms) { trj_render::draw_frame(si, atoms); });
}

int main() {
  test_trj();
}