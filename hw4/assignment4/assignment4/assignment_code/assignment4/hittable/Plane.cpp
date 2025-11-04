#include "Plane.hpp"

namespace GLOO {
Plane::Plane(const glm::vec3& normal, float d) {
  normal_ = normal;
  d_ = -d;
}

bool Plane::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  // TODO: Implement ray-plane intersection.
  // check if plan is parallel to ray
  float epsilon = 1e-6;
  if (std::abs(glm::dot(ray.GetDirection(), glm::vec3(0,1,0))) < epsilon) {
    return false;
  }

  float denom = glm::dot(normal_, ray.GetDirection());
  float t = -(glm::dot(normal_, ray.GetOrigin()) + d_) / denom;

  if (t < t_min) {
    return false;
  }

  record.time = t;
  record.normal = normal_;

  return true;
}
}  // namespace GLOO
