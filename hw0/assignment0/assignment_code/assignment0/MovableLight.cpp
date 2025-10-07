#include "MovableLight.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/string_cast.hpp"


#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/cameras/BasicCameraNode.hpp"
#include "gloo/InputManager.hpp"


// More include here
namespace GLOO {
MovableLight::MovableLight() {
// Constructor

// moved from MeshViewerApp.cpp

    auto point_light = std::make_shared<PointLight>();
    point_light->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
    point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
    point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
    CreateComponent<LightComponent>(point_light);
    GetTransform().SetPosition(glm::vec3(0.0f, 4.0f, 5.f));
}
void MovableLight::Update(double delta_time) {
// Update
    float speed = 5.0f;
    glm::vec3 position = GetTransform().GetPosition();
    glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
    if (InputManager::GetInstance().IsKeyPressed(262)){
        translation.x += speed * delta_time;
    }
    if (InputManager::GetInstance().IsKeyPressed(263)){
        translation.x -= speed * delta_time;
    }
    if (InputManager::GetInstance().IsKeyPressed(265)){
        translation.y += speed * delta_time;
    }
    if (InputManager::GetInstance().IsKeyPressed(264)){
        translation.y -= speed * delta_time;
    }

    position += translation;
    GetTransform().SetPosition(position);
}

}
