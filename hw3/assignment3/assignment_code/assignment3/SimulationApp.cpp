#include "SimulationApp.hpp"

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
#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/debug/AxisNode.hpp"
#include "ParticleState.hpp"
#include "SimpleSystemNode.hpp"
#include "SimpleSystem.hpp"
#include "IntegratorFactory.hpp"
#include "PendulumSystem.hpp"
#include "PendulumSystemNode.hpp"
#include "ClothSystemNode.hpp"


namespace GLOO {
SimulationApp::SimulationApp(const std::string& app_name,
                             glm::ivec2 window_size,
                             IntegratorType integrator_type,
                             float integration_step)
    : Application(app_name, window_size),
      integrator_type_(integrator_type),
      integration_step_(integration_step) {
  // TODO: remove the following two lines and use integrator type and step to
  // create integrators; the lines below exist only to suppress compiler
  // warnings.
  // UNUSED(integrator_type_);
  // UNUSED(integration_step_);
  integrator_type_ = integrator_type;
  integration_step_ = integration_step;
}

void SimulationApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  auto camera_node = make_unique<ArcBallCameraNode>(45.f, 0.75f, 5.0f);
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  root.AddChild(make_unique<AxisNode>('A'));

  auto ambient_light = std::make_shared<AmbientLight>();
  ambient_light->SetAmbientColor(glm::vec3(0.5f));
  root.CreateComponent<LightComponent>(ambient_light);

  auto point_light = std::make_shared<PointLight>();
  point_light->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
  point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
  auto point_light_node = make_unique<SceneNode>();
  point_light_node->CreateComponent<LightComponent>(point_light);
  point_light_node->GetTransform().SetPosition(glm::vec3(0.0f, 2.0f, 4.f));
  root.AddChild(std::move(point_light_node)); 

  auto point_light2 = std::make_shared<PointLight>();
  point_light2->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
  point_light2->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  point_light2->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
  auto point_light_node2 = make_unique<SceneNode>();
  point_light_node2->CreateComponent<LightComponent>(point_light2);
  point_light_node2->GetTransform().SetPosition(glm::vec3(3.0f, 5.0f, 3.0f));
  root.AddChild(std::move(point_light_node2));  

  // simple state 1 particle
  ParticleState simple_state;
  simple_state.positions.push_back(glm::vec3(5.0f, 0.0f, 5.0f));
  simple_state.velocities.push_back(glm::vec3(0.0f));

  // Create an integrator and pass ownership to the system node.
  auto integrator = IntegratorFactory::CreateIntegrator<SimpleSystem, ParticleState>(integrator_type_);
  auto simplesys_node = make_unique<SimpleSystemNode>(std::move(integrator), integration_step_);
  root.AddChild(std::move(simplesys_node));

  // pendulum system with 2 particles

  auto pendulum_integrator = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(integrator_type_);
  auto pendulumsys_node_ptr = make_unique<PendulumSystemNode>(std::move(pendulum_integrator), integration_step_);
  root.AddChild(std::move(pendulumsys_node_ptr));

  auto cloth_integrator = IntegratorFactory::CreateIntegrator<PendulumSystem, ParticleState>(integrator_type_);
  auto clothsys_node_ptr = make_unique<ClothSystemNode>(std::move(cloth_integrator), integration_step_, 10, 10);
  clothsys_node_ptr->GetTransform().SetPosition(glm::vec3(0.0f, 5.0f, -5.0f));
  root.AddChild(std::move(clothsys_node_ptr));

}
}  // namespace GLOO
