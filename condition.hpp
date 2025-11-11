#pragma once
#include "vector3d.hpp"
#include <lammpstrj/lammpstrj.hpp>

namespace trj_render {

class Condition {
public:
  virtual ~Condition() = default;
  virtual bool check(lammpstrj::Atom &atom) = 0;
};

class XMinCondition : public Condition {
public:
  explicit XMinCondition(double x_min) : x_min_(x_min) {}
  bool check(lammpstrj::Atom &atom) override {
    return atom.x > x_min_;
  }

private:
  double x_min_;
};

class XMaxCondition : public Condition {
public:
  explicit XMaxCondition(double x_max) : x_max_(x_max) {}
  bool check(lammpstrj::Atom &atom) override {
    return atom.x < x_max_;
  }

private:
  double x_max_;
};

class YMinCondition : public Condition {
public:
  explicit YMinCondition(double y_min) : y_min_(y_min) {}
  bool check(lammpstrj::Atom &atom) override {
    return atom.y > y_min_;
  }

private:
  double y_min_;
};

class YMaxCondition : public Condition {
public:
  explicit YMaxCondition(double y_max) : y_max_(y_max) {}
  bool check(lammpstrj::Atom &atom) override {
    return atom.y < y_max_;
  }

private:
  double y_max_;
};

class ZMinCondition : public Condition {
public:
  explicit ZMinCondition(double z_min) : z_min_(z_min) {}
  bool check(lammpstrj::Atom &atom) override {
    return atom.z > z_min_;
  }

private:
  double z_min_;
};

class ZMaxCondition : public Condition {
public:
  explicit ZMaxCondition(double z_max) : z_max_(z_max) {}
  bool check(lammpstrj::Atom &atom) override {
    return atom.z < z_max_;
  }

private:
  double z_max_;
};
}