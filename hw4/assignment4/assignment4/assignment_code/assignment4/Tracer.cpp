#include "Tracer.hpp"

#include <glm/gtx/string_cast.hpp>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include "gloo/Transform.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/lights/AmbientLight.hpp"

#include "gloo/Image.hpp"
#include "Illuminator.hpp"

namespace GLOO {
void Tracer::Render(const Scene& scene, const std::string& output_file) {
  scene_ptr_ = &scene;

  auto& root = scene_ptr_->GetRootNode();
  tracing_components_ = root.GetComponentPtrsInChildren<TracingComponent>();
  light_components_ = root.GetComponentPtrsInChildren<LightComponent>();


  Image image(image_size_.x, image_size_.y);

  std::cout << "Starting render..." << std::endl;
  std::cout << "Image size: " << image_size_.x << " x " << image_size_.y << std::endl;
  const float W = float(image_size_.x);
  const float H = float(image_size_.y);
  const float aspect = W / H;
  for (size_t y = 0; y < image_size_.y; y++) {
    for (size_t x = 0; x < image_size_.x; x++) {
      // Map pixel coordinates to a centered image plane in [-1,1].
      // Pixel centers in NDC
      float u = (float(x) + 0.5f) / W;      // [0,1]
      float v = (float(y) + 0.5f) / H;      // [0,1]

      // Map to camera image plane
      float sx = (2.0f * u - 1.0f) * aspect;  // [-aspect, +aspect]
      float sy = -(1.0f - 2.0f * v);             // [-1, +1], +Y is up


      Ray ray = camera_.GenerateRay({sx, sy});
      HitRecord record;
      record.time = std::numeric_limits<float>::infinity();
      glm::vec3 color = TraceRay(ray, max_bounces_, record);
      // somehow set image to this color, figure out later
      image.SetPixel(x, y, color);

    }
  }

  if (output_file.size())
    image.SavePNG(output_file);
}


glm::vec3 Tracer::TraceRay(const Ray& ray,
                           size_t bounces,
                           HitRecord& record) const {
  // TODO: Compute the color for the cast ray.

  // do cases of bouncing/reflection later
  // std::cout << "rendering ray: " << glm::to_string(ray.GetOrigin()) << " dir: " << glm::to_string(ray.GetDirection()) << std::endl;

  bool hit_something_local = false;
  TracingComponent* closest_obj = nullptr;

  float closest_world_dist = std::numeric_limits<float>::infinity();
  HitRecord best_record;
  glm::vec3 best_world_hit(0.0f);

  // scan over all objects to find closest intersection measured in world space
  for (TracingComponent* obj : tracing_components_) {
    SceneNode* node = obj->GetNodePtr();
    glm::mat4 local_to_world = node->GetTransform().GetLocalToWorldMatrix();
    glm::mat4 world_to_local = glm::inverse(local_to_world);
    const HittableBase& hittable = obj->GetHittable();

    Ray local_ray = ray;
    local_ray.ApplyTransform(world_to_local);

    HitRecord tmp;
    if (hittable.Intersect(local_ray, 0.001f, tmp)) {
      // compute world-space hit and distance to camera
      glm::vec3 local_hit = local_ray.At(tmp.time);
      glm::vec3 world_hit = glm::vec3(local_to_world * glm::vec4(local_hit, 1.0f));
      glm::vec3 cam_pos = ray.GetOrigin();
      float world_dist = glm::length(world_hit - cam_pos);

      if (world_dist < closest_world_dist) {
        closest_world_dist = world_dist;
        closest_obj = obj;
        hit_something_local = true;
        // transform normal back to world space using inverse-transpose
        glm::mat3 normal_mat = glm::mat3(glm::transpose(world_to_local));
        tmp.normal = glm::normalize(normal_mat * tmp.normal);
        best_record = tmp;
        best_world_hit = world_hit;
      }
    }
  }

  if (hit_something_local) {
    // std::cout << "Closest hit at time: " << best_record.time << " with normal: " << glm::to_string(best_record.normal) << std::endl;
    glm::vec3 collision_point = best_world_hit;
    glm::vec3 diffuse_color = glm::vec3(0,0,0);
    auto mat_comp = closest_obj->GetNodePtr()->GetComponentPtr<MaterialComponent>();
    for (LightComponent* light_component : light_components_) {
      glm::vec3 dir_to_light(0.0f);
      glm::vec3 intensity(0.0f);
      float dist_to_light = 0.0f;
      Illuminator::GetIllumination(*light_component, collision_point, dir_to_light, intensity, dist_to_light);
      float coeff = std::max(0.0f, glm::dot(best_record.normal, dir_to_light));
      glm::vec3 mat_diffuse = mat_comp ? mat_comp->GetMaterial().GetDiffuseColor() : glm::vec3(1.0f);
      diffuse_color += intensity * coeff * mat_diffuse;
    }
    // std::cout << "Diffuse color: " << glm::to_string(diffuse_color) << std::endl;

    glm::vec3 specular_color(0.0f);
    glm::vec3 view_dir = glm::normalize(-ray.GetDirection());
    glm::vec3 mat_spec = mat_comp ? mat_comp->GetMaterial().GetSpecularColor() : glm::vec3(0.0f);
    float shininess = mat_comp ? mat_comp->GetMaterial().GetShininess() : 1.0f;
    for (LightComponent* light_component : light_components_) {
      glm::vec3 dir_to_light(0.0f);
      glm::vec3 intensity(0.0f);
      float dist_to_light = 0.0f;
      Illuminator::GetIllumination(*light_component, collision_point, dir_to_light, intensity, dist_to_light);
      glm::vec3 reflect_dir = glm::reflect(-dir_to_light, best_record.normal);
      float spec_coeff = pow(std::max(glm::dot(view_dir, reflect_dir), 0.0f), shininess);
      specular_color += intensity * spec_coeff * mat_spec;
    }
    // std::cout << "Specular color: " << glm::to_string(specular_color) << std::endl;
    glm::vec3 ambient_color = glm::vec3(0,0,0); // scene ambient currently unused
    glm::vec3 total_color = diffuse_color + specular_color + ambient_color;
    record = best_record;
    return total_color;
  }

  // std::cout << "missed\n";

  // if ray missed everything, return background color
  return GetBackgroundColor(ray.GetDirection());
}

// glm::vec3 Tracer::PhongDiffuse(const LightComponent& light_component, const HitRecord& record, const Ray& ray) const {
// // (max of (0, L_dir*N_dir)) *k_dif*L_intensity

// glm::vec3 dir_to_light = glm::vec3((0,0,0));
// glm::vec3 intensity = glm::vec3((0,0,0));
// float dist_to_light = 0;
// Illuminator::GetIllumination(light_component, ray.At(record.time), dir_to_light, intensity, dist_to_light);

// float coeff = std::max(0.0f, glm::dot(record.normal, dir_to_light));
// return intensity * coeff * light_component.GetNodePtr()->GetComponentPtr<MaterialComponent>()->GetMaterial().GetDiffuseColor();

// }
glm::vec3 Tracer::GetBackgroundColor(const glm::vec3& direction) const {
  if (cube_map_ != nullptr) {
    return cube_map_->GetTexel(direction);
  } else
    return background_color_;
}
}  // namespace GLOO
