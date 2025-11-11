#include "renderer.hpp"
#include <cstdio>
#include <cxxopts.hpp>
#include <lammpstrj/lammpstrj.hpp>

auto parse_argument(int argc, char **argv) {
  cxxopts::Options options("trj2png", "Render LAMMPS .lammpstrj frames to PNG (2D projection).");
  options.add_options()("x,rx", "Rotation around X axis (degrees)", cxxopts::value<double>()->default_value("0"));
  options.add_options()("y,ry", "Rotation around Y axis (degrees)", cxxopts::value<double>()->default_value("0"));
  options.add_options()("z,rz", "Rotation around Z axis (degrees)", cxxopts::value<double>()->default_value("0"));
  options.add_options()("s,scale", "Scale factor for simulation box → pixels (if negative, the scale is automatically adjusted so that the larger side of the image becomes 800 pixels)", cxxopts::value<double>()->default_value("-1"));
  options.add_options()("f,frame", "Render only this frame index (0-based). If omitted, renderall.", cxxopts::value<int>()->default_value("-1"));
  options.add_options()("h,help", "Showhelp");
  options.add_options("positional")("filename", "LAMMPS trajectory file (.lammpstrj)", cxxopts::value<std::string>());
  options.parse_positional({"filename"});

  options.add_options()("xmin", "Minimum x-coordinate to display", cxxopts::value<double>())("xmax", "Maximum x-coordinate to display", cxxopts::value<double>())("ymin", "Minimum y-coordinate to display", cxxopts::value<double>())("ymax", "Maximum y-coordinate to display", cxxopts::value<double>())("zmin", "Minimum z-coordinate to display", cxxopts::value<double>())("zmax", "Maximum z-coordinate to display", cxxopts::value<double>());

  for (int i = 0; i < trj_render::MAX_ATOM_TYPES; ++i) {
    std::string key = "radius" + std::to_string(i);
    std::string desc = "Radius of atom type " + std::to_string(i);
    options.add_options()(key, desc, cxxopts::value<double>());
  }

  auto result = options.parse(argc, argv);
  if (result.count("help")) {
    std::cout << options.help({"", "positional"}) << std::endl;
    exit(0);
  }

  if (!result.count("filename")) {
    std::cerr << "Error: filename is required.\n\n"
              << options.help({"", "positional"}) << std::endl;
    exit(1);
  }
  return result;
}

void read_lammpstrj(int argc, char **argv) {
  auto result = parse_argument(argc, argv);

  const std::string filename = result["filename"].as<std::string>();

  std::ifstream fin(filename.c_str());
  if (!fin.good()) {
    std::cerr << "Error: File not found: " << filename << std::endl;
    std::exit(1);
  }

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
  if (result.count("xmin")) {
    double xmin = result["xmin"].as<double>();
    renderer.add_condition(std::make_unique<trj_render::XMinCondition>(xmin));
  }
  if (result.count("xmax")) {
    double xmax = result["xmax"].as<double>();
    renderer.add_condition(std::make_unique<trj_render::XMaxCondition>(xmax));
  }
  if (result.count("ymin")) {
    double ymin = result["ymin"].as<double>();
    renderer.add_condition(std::make_unique<trj_render::YMinCondition>(ymin));
  }
  if (result.count("ymax")) {
    double ymax = result["ymax"].as<double>();
    renderer.add_condition(std::make_unique<trj_render::YMaxCondition>(ymax));
  }
  if (result.count("zmin")) {
    double zmin = result["zmin"].as<double>();
    renderer.add_condition(std::make_unique<trj_render::ZMinCondition>(zmin));
  }
  if (result.count("zmax")) {
    double zmax = result["zmax"].as<double>();
    renderer.add_condition(std::make_unique<trj_render::ZMaxCondition>(zmax));
  }

  for (int i = 0; i < trj_render::MAX_ATOM_TYPES; ++i) {
    std::string opt = "radius" + std::to_string(i);
    if (result.count(opt)) {
      double r = result[opt].as<double>();
      renderer.set_atom_radius(i, r);
    }
  }

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

void test() {
  trj_render::Vector3d b1(0, 0, 0);
  trj_render::Vector3d b2(20, 20, 20);
  trj_render::Projector proj(b1, b2);
  proj.rotateY(30);
  proj.rotateX(30);
  proj.setScale(-1);
  trj_render::Renderer renderer(proj);
  std::vector<lammpstrj::Atom> atoms;
  lammpstrj::Atom a;
  a.x = 10;
  a.y = 10;
  a.z = 10;
  a.type = 1;
  atoms.push_back(a);
  auto [width, height] = proj.canvas_size();
  trj_render::Canvas canvas(width, height);
  auto si = std::make_unique<lammpstrj::SystemInfo>();
  si->x_min = b1.x;
  si->y_min = b1.y;
  si->z_min = b1.z;
  si->x_max = b2.x;
  si->y_max = b2.y;
  si->z_max = b2.z;
  canvas.set_color(0, 0, 0);
  canvas.fill_rect(0, 0, width, height);
  renderer.set_atom_radius(1, 10);
  renderer.draw_simulation_box_back(si, canvas, proj);
  renderer.draw_atoms(atoms, canvas, proj);
  renderer.draw_simulation_box_front(si, canvas, proj);
  canvas.save("test.png");
}

int main(int argc, char **argv) {
  read_lammpstrj(argc, argv);
  // test();
}