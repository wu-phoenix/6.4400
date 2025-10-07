#include "TeapotNode.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/color_space.hpp"

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
TeapotNode::TeapotNode() {
// Constructor

// moved from MeshViewerApp.cpp

    std::shared_ptr<PhongShader> shader = std::make_shared<PhongShader>();
    std::shared_ptr<VertexObject> mesh =
        MeshLoader::Import("assignment0/teapot.obj").vertex_obj;
    if (mesh == nullptr) {
    return;
    }
    CreateComponent<ShadingComponent>(shader);
    CreateComponent<RenderingComponent>(mesh);
    GetTransform().SetPosition(glm::vec3(0.f, 0.f, 0.f));
    GetTransform().SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), 0.3f);
    CreateComponent<MaterialComponent>(
        std::make_shared<Material>(Material::GetDefault()));
}
void TeapotNode::Update(double delta_time) {
// Update

    static bool prev_released = true;
        if (InputManager::GetInstance().IsKeyPressed('C')) {
            if (prev_released) {
                std::cout << "C key pressed" << std::endl;
                ToggleColor();
                }
            prev_released = false;
        } else if (InputManager::GetInstance().IsKeyReleased('C')) {
            prev_released = true;
        }

    if(InputManager::GetInstance().IsKeyPressed('T')){
        TraverseColor(delta_time);
    }
}

void TeapotNode::ToggleColor() {
    static int color_index = 0;
    color_index = (color_index + 1) % 3;
    std::cout << "ToggleColor called" << std::endl;
    // Toggle between R,G,B
    std::vector<glm::vec3> colors = {glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),      
        glm::vec3(0.0f, 0.0f, 1.0f)};
    auto material_component = GetComponentPtr<MaterialComponent>();
    if (material_component) {
        auto& material = material_component->GetMaterial();
        material.SetAmbientColor(colors[color_index]);
        material.SetDiffuseColor(colors[color_index]);
        material.SetSpecularColor(glm::vec3(1.0f));
    }
}

void TeapotNode::TraverseColor(double delta_time){
    glm::vec3 hue = glm::hsvColor(GetComponentPtr<MaterialComponent>()->GetMaterial().GetAmbientColor());
    float hue_speed = 60.0f; // degrees per second

    hue += glm::vec3(hue_speed * delta_time, 0.0f, 0.0f); 
    if (hue[0] >= 360.0f){
        hue[0] -= 360.0f;
    } 
    
    glm::vec3 rgb = glm::rgbColor(hue); // HSV to RGB conversion

    auto material_component = GetComponentPtr<MaterialComponent>();
    if (material_component) {
        auto& material = material_component->GetMaterial();
        material.SetAmbientColor(rgb);
        material.SetDiffuseColor(rgb);
        material.SetSpecularColor(glm::vec3(1.0f));
    }
}

}