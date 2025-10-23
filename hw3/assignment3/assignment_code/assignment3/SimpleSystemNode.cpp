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
#include "SimpleSystemNode.hpp"
#include "gloo/shaders/PhongShader.hpp"


namespace GLOO {

SimpleSystemNode::SimpleSystemNode(
        std::unique_ptr<IntegratorBase<SimpleSystem, ParticleState>> integrator,
        float dt) {
    // this will be a shared mesh for all particles (aka the one particle for this system)
    
    integrator_ = std::move(integrator);
    dt_ = dt;
    time_ = 0.0f;
    sphere_mesh_ = PrimitiveFactory::CreateSphere(0.3f, 10, 10);
    shader_ = std::make_shared<PhongShader>();
    
    particle_state_ = ParticleState();
    particles_ = std::vector<SceneNode*>();
    particle_system_ = SimpleSystem();
    
    // Create a single particle state
    particle_state_.positions.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    particle_state_.velocities.push_back(glm::vec3(0.0f));
    particles_.resize(particle_state_.positions.size());
    
    // for each particle state, create a SceneNode that holds the mesh/visualization.
    // We'll constantly update this with the particle state.
    for (size_t i = 0; i < particle_state_.positions.size(); ++i) {
        auto particle_node = make_unique<SceneNode>();
        particle_node->GetTransform().SetPosition(particle_state_.positions[i]);
        particle_node->CreateComponent<RenderingComponent>(sphere_mesh_);
        particle_node->CreateComponent<ShadingComponent>(shader_);
        auto temp_pointer = particle_node.get();
        this->AddChild(std::move(particle_node));
        particles_[i] = temp_pointer;
    }
}

void SimpleSystemNode::Update(double dt) 
     {
    // std::cout << "updating simple system node" << std::endl;
        particle_state_ = integrator_->Integrate(particle_system_, particle_state_, time_, dt_);
        time_ += dt_;
        // update particle nodes to visualize
        for (size_t i = 0; i < particles_.size(); ++i) {
            particles_[i]->GetTransform().SetPosition(particle_state_.positions[i]);
        }
    }

}  // namespace GLOO