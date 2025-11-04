#include "Illuminator.hpp"

#include <limits>
#include <stdexcept>

#include <glm/geometric.hpp>

#include "gloo/lights/DirectionalLight.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/SceneNode.hpp"


namespace GLOO {
void Illuminator::GetIllumination(const LightComponent& light_component,
                                  const glm::vec3& hit_pos,
                                  glm::vec3& dir_to_light,
                                  glm::vec3& intensity,
                                  float& dist_to_light) {
  // Calculation will be done in world space.

  auto light_ptr = light_component.GetLightPtr();
  if (light_ptr->GetType() == LightType::Directional) {
    auto directional_light_ptr = static_cast<DirectionalLight*>(light_ptr);
    dir_to_light = -directional_light_ptr->GetDirection();
    intensity = directional_light_ptr->GetDiffuseColor();
    dist_to_light = std::numeric_limits<float>::max();
  } else if(light_ptr->GetType() == LightType::Point) {

    auto point_light_ptr = static_cast<PointLight*>(light_ptr);
    glm::vec3 attenuation = point_light_ptr->GetAttenuation(); // (constant, linear, quadratic)
  // direction from hit point toward the light position
  dir_to_light = light_component.GetNodePtr()->GetTransform().GetPosition() - hit_pos;
  dist_to_light = glm::length(dir_to_light);
  if (dist_to_light > 0.0f) dir_to_light /= dist_to_light;

    // don't know if this is how i'm meant to do this?
    intensity = point_light_ptr->GetDiffuseColor() /
                (attenuation.x +
                 attenuation.y * dist_to_light +
                 attenuation.z * dist_to_light * dist_to_light);

  } else if(light_ptr->GetType() == LightType::Ambient) {
    auto ambient_light_ptr = static_cast<AmbientLight*>(light_ptr);
    dir_to_light = glm::vec3(0,0,0); // no direction for ambient light
    intensity = ambient_light_ptr->GetAmbientColor();
    dist_to_light = std::numeric_limits<float>::max();

  }else {
    throw std::runtime_error(
        "Unrecognized light type when computing "
        "illumination");
  }
}
}  // namespace GLOO
