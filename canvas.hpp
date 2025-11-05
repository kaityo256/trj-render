#pragma once

#include "vector3d.hpp"
#include <fstream>
#include <iostream>
#include <lodepng.h>
#include <vector>
namespace trj_render {

typedef unsigned char BYTE;

class canvas {
private:
  int width, height, line;
  int cx, cy;   // Current Point
  BYTE R, G, B; // Current Color

public:
  std::vector<BYTE> image_buffer;
  canvas(int w, int h) {
    width = w;
    height = h;
    line = w * 4;
    image_buffer.resize(w * h * 4, 255);
    cx = 0;
    cy = 0;
  }

  void moveto(const trj_render::Vector2d &v) {
    int ix = static_cast<int>(v.x);
    int iy = static_cast<int>(v.y);
    moveto(ix, iy);
  }

  void moveto(int x, int y) {
    cx = x;
    cy = y;
  }

  void lineto(const trj_render::Vector2d &v) {
    int ix = static_cast<int>(v.x);
    int iy = static_cast<int>(v.y);
    lineto(ix, iy);
  }

  void lineto(int x, int y) {
    int dx = (x > cx) ? x - cx : cx - x;
    int dy = (y > cy) ? y - cy : cy - y;

    int sx = (x > cx) ? 1 : -1;
    int sy = (y > cy) ? 1 : -1;

    if (dx > dy) {
      int E = -dx;
      for (int i = 0; i <= dx; i++) {
        draw_point(cx, cy);
        cx += sx;
        E += 2 * dy;
        if (E >= 0) {
          cy += sy;
          E -= 2 * dx;
        }
      }
    } else {
      int E = -dy;
      for (int i = 0; i <= dy; i++) {
        draw_point(cx, cy);
        cy += sy;
        E += 2 * dx;
        if (E >= 0) {
          cx += sx;
          E -= 2 * dy;
        }
      }
    }
  }

  void draw_point(int x, int y) {
    if (x < 0 || x >= width)
      return;
    if (y < 0 || y >= height)
      return;
    int p = y * line + x * 4;
    image_buffer[p] = R;
    image_buffer[p + 1] = G;
    image_buffer[p + 2] = B;
  }

  void set_color(BYTE red, BYTE green, BYTE blue) {
    R = red;
    G = green;
    B = blue;
  }

  void draw_rect(int x, int y, int w, int h) {
    for (int iy = y; iy <= y + h; iy++) {
      draw_point(x, iy);
      draw_point(x + w, iy);
    }
    for (int ix = x; ix <= x + w; ix++) {
      draw_point(ix, y);
      draw_point(ix, y + h);
    }
  }

  void fill_rect(int x, int y, int w, int h) {
    for (int iy = 0; iy < h; iy++) {
      for (int ix = 0; ix < w; ix++) {
        draw_point(ix + x, iy + y);
      }
    }
  }

  void fill_circle(int x0, int y0, int r) {
    int x = r;
    int y = 0;
    int F = -2 * r + 3;
    while (x >= y) {
      for (int i = -x; i <= x; i++) {
        draw_point(x0 + i, y0 + y);
        draw_point(x0 + i, y0 - y);
        draw_point(x0 + y, y0 + i);
        draw_point(x0 - y, y0 + i);
      }
      if (F >= 0) {
        x--;
        F -= 4 * x;
      }
      y++;
      F += 4 * y + 2;
    }
  }

  void draw_circle(int x0, int y0, int r) {
    int x = r;
    int y = 0;
    int F = -2 * r + 3;
    while (x >= y) {
      draw_point(x0 + x, y0 + y);
      draw_point(x0 - x, y0 + y);
      draw_point(x0 + x, y0 - y);
      draw_point(x0 - x, y0 - y);
      draw_point(x0 + y, y0 + x);
      draw_point(x0 - y, y0 + x);
      draw_point(x0 + y, y0 - x);
      draw_point(x0 - y, y0 - x);
      if (F >= 0) {
        x--;
        F -= 4 * x;
      }
      y++;
      F += 4 * y + 2;
    }
  }

  void save(const char *filename) {
    lodepng::encode(filename, image_buffer, width, height);
  }
};
} // namespace trj_render
