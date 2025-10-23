#include "gloo/SceneNode.hpp"
#include "gloo/utils.hpp"
#include "ForwardEulerIntegrator.hpp"
#include "SimpleSystem.hpp"
#include "ParticleState.hpp"
#include "ParticleSystemBase.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "PendulumSystemNode.hpp"
#include "gloo/shaders/PhongShader.hpp"


namespace GLOO {

PendulumSystemNode::PendulumSystemNode(
        std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator,
        float dt) {
    // this will be a shared mesh for all particles (aka the one particle for this system)
    
    integrator_ = std::move(integrator);
    dt_ = dt;
    time_ = 0.0f;
    sphere_mesh_ = PrimitiveFactory::CreateSphere(0.1f, 10, 10);
    
    shader_ = std::make_shared<PhongShader>();
    
    particle_state_ = ParticleState();
    // Create a single spring state
    particle_state_.positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    particle_state_.velocities.push_back(glm::vec3(0.0f));
    particle_state_.positions.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
    particle_state_.velocities.push_back(glm::vec3(-1.0f));
    //create additional 2 particles
    particle_state_.positions.push_back(glm::vec3(0.0f, 0.0f, -2.0f));
    particle_state_.velocities.push_back(glm::vec3(-1.0f));
    particle_state_.positions.push_back(glm::vec3(0.0f, 0.0f, -3.0f));
    particle_state_.velocities.push_back(glm::vec3(-1.0f));



    // set parameters of the pendulum system like gravity
    particle_system_ = PendulumSystem();
    particle_system_.SetFixedIndices({0});
    particle_system_.SetMasses({1.0f, 1.0f, 1.0f, 1.0f});
    particle_system_.SetSprings({glm::vec4(0, 1, 1.0f, 50.0f), glm::vec4(1, 2, 1.0f, 50.0f), glm::vec4(2, 3, 1.0f, 50.0f)});
    particle_system_.SetDragCoefficient(0.1f);
    particles_.resize(particle_state_.positions.size());

    particles_ = std::vector<SceneNode*>();

    // for each particle state, create a SceneNode that holds the mesh/visualization.
    // We'll constantly update this with the particle state.
    for (size_t i = 0; i < particle_state_.positions.size(); ++i) {
        auto particle_node = make_unique<SceneNode>();
        particle_node->GetTransform().SetPosition(particle_state_.positions[i]);
        particle_node->CreateComponent<RenderingComponent>(sphere_mesh_);
        particle_node->CreateComponent<ShadingComponent>(shader_);
        // auto temp_pointer = particle_node.get();
        this->AddChild(std::move(particle_node));
    }
}

void PendulumSystemNode::Update(double dt) 
     {
    // std::cout << "updating pendulum system node" << std::endl;
        particle_state_ = integrator_->Integrate(particle_system_, particle_state_, time_, dt_);
        time_ += dt_;
        // update particle nodes to visualize
        for (size_t i = 0; i < this->GetChildrenCount(); ++i) {
            this->GetChild(i).GetTransform().SetPosition(particle_state_.positions[i]);
        }
    }

}  // namespace GLOO