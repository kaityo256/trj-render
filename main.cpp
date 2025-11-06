#include "renderer.hpp"
#include <cstdio>
#include <cxxopts.hpp>
#include <iostream>
#include <lammpstrj/lammpstrj.hpp>

int main(int argc, char **argv) {
  cxxopts::Options options("trj2png", "Render LAMMPS .lammpstrj frames to PNG (2D projection).");
  options.add_options()("x,rx", "Rotation around X axis (degrees)", cxxopts::value<double>()->default_value("0"));
  options.add_options()("y,ry", "Rotation around Y axis (degrees)", cxxopts::value<double>()->default_value("0"));
  options.add_options()("z,rz", "Rotation around Z axis (degrees)", cxxopts::value<double>()->default_value("0"));
  options.add_options()("s,scale", "Scale factor for simulation box → pixels (if negative, the scale is automatically adjusted so that the larger side of the image becomes 800 pixels)", cxxopts::value<double>()->default_value("-1"));
  options.add_options()("f,frame", "Render only this frame index (0-based). If omitted, renderall.", cxxopts::value<int>()->default_value("-1"));
  options.add_options()("h,help", "Showhelp");
  options.add_options("positional")("filename", "LAMMPS trajectory file (.lammpstrj)", cxxopts::value<std::string>());
  options.parse_positional({"filename"});

  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help({"", "positional"}) << std::endl;
    return 0;
  }

  if (!result.count("filename")) {
    std::cerr << "Error: filename is required.\n\n"
              << options.help({"", "positional"}) << std::endl;
    return 1;
  }

  const std::string filename = result["filename"].as<std::string>();
  const double rx_deg = result["rx"].as<double>();
  const double ry_deg = result["ry"].as<double>();
  const double rz_deg = result["rz"].as<double>();
  const double scale = result["scale"].as<double>();
  const int frame_index = result["frame"].as<int>();

  auto si = lammpstrj::read_info(filename);
  trj_render::Vector3d b1(si->x_min, si->y_min, si->z_min);
  trj_render::Vector3d b2(si->x_max, si->y_max, si->z_max);
  trj_render::Projector proj(b1, b2);
  proj.rotateX(rx_deg);
  proj.rotateY(ry_deg);
  proj.rotateZ(rz_deg);
  proj.setScale(scale);
  trj_render::Renderer renderer(proj);

  if (frame_index < 0) {
    lammpstrj::for_each_frame(filename,
                              [&renderer](const auto &si, auto &atoms) {
                                renderer.draw_frame(si, atoms);
                              });
  } else {
    lammpstrj::for_frame(frame_index, filename,
                         [&renderer](const auto &si, auto &atoms) {
                           renderer.draw_frame(si, atoms);
                         });
  }
}