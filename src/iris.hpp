#ifndef _IRIS_H
#define _IRIS_H

#include <Eigen/Core>
#include <vector>
#include <memory>
#include <iostream>


namespace iris {

const double ELLIPSOID_C_EPSILON = 1e-4;

struct IRISOptions {
  bool require_containment = false; // if true: if the IRIS polyhedron no longer contains the seed point
  bool error_on_infeasible_start = false;
  double termination_threshold = 2e-2;
  int iter_limit = 100;
};

class Polyhedron {
public:
  Polyhedron(int dim=0);
  Polyhedron(Eigen::MatrixXd A, Eigen::VectorXd b);
  ~Polyhedron() {
    // std::cout << "deleting polyhedron: " << this << std::endl;
  }
  void setA(const Eigen::MatrixXd &A);
  const Eigen::MatrixXd& getA() const;
  void setB(const Eigen::VectorXd &b);
  const Eigen::VectorXd& getB() const;
  int getDimension() const;
  int getNumberOfConstraints() const;
  void appendConstraints(const Polyhedron &other);
  std::vector<Eigen::VectorXd> generatorPoints();
  std::vector<Eigen::VectorXd> generatorRays();
  bool contains(Eigen::VectorXd point, double tolerance=0.0);

private:
  Eigen::MatrixXd A_;
  Eigen::VectorXd b_;
  bool dd_representation_dirty_ = true;
  std::vector<Eigen::VectorXd> generator_points_;
  std::vector<Eigen::VectorXd> generator_rays_;
  void updateDDRepresentation();
};



class Ellipsoid {
public:
  Ellipsoid(int dim=0);
  Ellipsoid(Eigen::MatrixXd C, Eigen::VectorXd d);
  ~Ellipsoid() {
    // std::cout << "deleting ellipsoid: " << this << std::endl;
  }
  const Eigen::MatrixXd& getC() const;
  const Eigen::VectorXd& getD() const;
  void setC(const Eigen::MatrixXd &C_);
  void setCEntry(Eigen::DenseIndex row, Eigen::DenseIndex col, double value);
  void setD(const Eigen::VectorXd &d_);
  void setDEntry(Eigen::DenseIndex idx, double value);
  int getDimension() const;
  static std::shared_ptr<Ellipsoid> fromNSphere(Eigen::VectorXd &center, double radius=ELLIPSOID_C_EPSILON);
  double getVolume() const;

private:
  Eigen::MatrixXd C_;
  Eigen::VectorXd d_;
};

class IRISRegion {
public:
  std::shared_ptr<Polyhedron> polyhedron;
  std::shared_ptr<Ellipsoid> ellipsoid;

  IRISRegion(int dim=0) {
    polyhedron.reset(new Polyhedron(dim));
    ellipsoid.reset(new Ellipsoid(dim));
  }
};

struct IRISDebugData {
  std::vector<Ellipsoid> ellipsoid_history;
  std::vector<Polyhedron> polyhedron_history;
  std::vector<Eigen::MatrixXd> obstacles;
  Polyhedron bounds;
  // Eigen::VectorXd ellipsoid_times;
  // Eigen::VectorXd polyhedron_times;
  // double total_time;
  int iters;
};

class IRISProblem {
private:
  std::vector<Eigen::MatrixXd> obstacle_pts; // each obstacle is a matrix of size (_dim, pts_per_obstacle)
  Polyhedron bounds;
  int dim;
  Ellipsoid seed;

public:
  IRISProblem(int dim):
    bounds(dim),
    dim(dim),
    seed(dim) {}

  void setSeedPoint(Eigen::VectorXd point);
  void setSeedEllipsoid(Ellipsoid ellipsoid);
  int getDimension() const;
  Ellipsoid getSeed() const;
  void setBounds(Polyhedron new_bounds);
  void addObstacle(Eigen::MatrixXd new_obstacle_vertices);
  const std::vector<Eigen::MatrixXd>& getObstacles() const;
  Polyhedron getBounds() const;
};


std::shared_ptr<IRISRegion> inflate_region(const IRISProblem &problem, const IRISOptions &options, IRISDebugData *debug=NULL);

void separating_hyperplanes(const std::vector<Eigen::MatrixXd> obstacle_pts, const Ellipsoid &ellipsoid, Polyhedron &polyhedron, bool &infeasible_start);

void getGenerators(const Eigen::MatrixXd &A, const Eigen::VectorXd &b, std::vector<Eigen::VectorXd> &points, std::vector<Eigen::VectorXd> &rays);

}

#endif