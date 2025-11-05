#include "canvas.hpp"
#include "projector.hpp"
#include "vector3d.hpp"
#include <iostream>

void test() {
  // Prepare a canvas (width, height)
  trj_render::canvas canvas(256, 256);

  // Draw a filled rectangle
  canvas.set_color(255, 255, 255);
  canvas.fill_rect(0, 0, 256, 256);

  // Draw a filled circle
  canvas.set_color(255, 0, 0);
  canvas.fill_circle(128, 128, 32);

  // Draw a line
  canvas.set_color(0, 0, 0);
  canvas.moveto(0, 0);
  canvas.lineto(256, 256);

  // Draw a rectangle
  canvas.set_color(0, 0, 255);
  canvas.draw_rect(32, 32, 64, 32);

  canvas.save("test.png");
}

void test3d() {
  std::vector<trj_render::Vector3d> atoms;
  const double L = 20.0;
  int is = static_cast<int>(L);
  for (int ix = -is; ix < is; ix++) {
    for (int iy = -is; iy < is; iy++) {
      for (int iz = -is; iz < is; iz++) {
        double x, y, z;
        x = ix;
        y = iy;
        z = iz;
        if (x * x + y * y + z * z < L * L)
          atoms.push_back(trj_render::Vector3d(x, y, z));
        x = ix + 0.5;
        y = iy + 0.5;
        z = iz;
        if (x * x + y * y + z * z < L * L)
          atoms.push_back(trj_render::Vector3d(x, y, z));
        x = ix;
        y = iy + 0.5;
        z = iz + 0.5;
        if (x * x + y * y + z * z < L * L)
          atoms.push_back(trj_render::Vector3d(x, y, z));
        x = ix + 0.5;
        y = iy;
        z = iz + 0.5;
        if (x * x + y * y + z * z < L * L)
          atoms.push_back(trj_render::Vector3d(x, y, z));
      }
    }
  }
  std::vector<std::size_t> idx(atoms.size());
  for (std::size_t i = 0; i < atoms.size(); ++i) {
    idx[i] = i;
  }
  trj_render::Vector3d b1 = trj_render::Vector3d(-L, -L, -L);
  trj_render::Vector3d b2 = trj_render::Vector3d(L, L, L);

  trj_render::Projector proj(b1, b2);
  proj.setScale(10);
  proj.rotateY(45);
  proj.rotateZ(30);
  std::pair<double, double> size = proj.canvas_size();
  double width = size.first;
  double height = size.second;
  printf("%f %f\n", width, height);
  trj_render::canvas canvas(width, height);
  canvas.set_color(255, 0, 0);
  std::sort(idx.begin(), idx.end(),
            [&](std::size_t ia, std::size_t ib) {
              double da = proj.depth(atoms[ia]);
              double db = proj.depth(atoms[ib]);
              return da < db;
            });
  for (std::size_t i : idx) {
    trj_render::Vector2d s = proj.project2d(atoms[i]);
    canvas.set_color(255, 0, 0);
    canvas.fill_circle(s.x, s.y, 5);
    canvas.set_color(0, 0, 0);
    canvas.draw_circle(s.x, s.y, 5);
  }
  proj.draw_simulation_box(canvas);
  canvas.save("test.png");
}

int main() {
  test3d();
}