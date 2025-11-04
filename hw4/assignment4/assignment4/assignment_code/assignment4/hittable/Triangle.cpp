#include "Triangle.hpp"

#include <iostream>
#include <stdexcept>

#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Plane.hpp"

namespace GLOO {
Triangle::Triangle(const glm::vec3& p0,
                   const glm::vec3& p1,
                   const glm::vec3& p2,
                   const glm::vec3& n0,
                   const glm::vec3& n1,
                   const glm::vec3& n2) {
  positions_ = {p0, p1, p2};
  normals_ = {n0, n1, n2};
}

Triangle::Triangle(const std::vector<glm::vec3>& positions,
                   const std::vector<glm::vec3>& normals) {
  positions_ = positions;
  normals_ = normals;
}

bool Triangle::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  // TODO: Implement ray-triangle intersection.

  // line equations e1, e2
  glm::vec3 e1 = positions_[1] - positions_[0];
  glm::vec3 e2 = positions_[2] - positions_[0];

  glm::vec3 h = glm::cross(ray.GetDirection(), e2);
  float determinant = glm::dot(e1, h);

  if (determinant > -1e-8 && determinant < 1e-8) {
    return false;
  }

  float inv_determinant = 1.0f / determinant;

  glm::vec3 s = ray.GetOrigin() - positions_[0];
  float u = inv_determinant * glm::dot(s, h);

  if (u < 0.0f || u > 1.0f) {
    return false;
  }

  glm::vec3 q = glm::cross(s, e1);
  float v = inv_determinant * glm::dot(ray.GetDirection(), q);

  if (v < 0.0f || u + v > 1.0f) {
    return false;
  }

  float t = inv_determinant * glm::dot(e2, q);

  if (t < t_min) {
    return false;
  }

  if (t >= record.time) {
    return false;
  }
  record.time = t;
  record.normal = glm::normalize((1 - u - v) * normals_[0] + u * normals_[1] + v * normals_[2]);
  return true;
}
  
}  // namespace GLOO
