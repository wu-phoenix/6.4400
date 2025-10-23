#pragma once

#include "gloo/SceneNode.hpp"
#include "gloo/utils.hpp"
#include "ForwardEulerIntegrator.hpp"
#include "PendulumSystem.hpp"
#include "ParticleState.hpp"
#include "ParticleSystemBase.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"


namespace GLOO {

class PendulumSystemNode : public SceneNode {
 public:
    PendulumSystemNode(std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator,
                                     float dt);

    void Update(double dt) override;
    void AddParticleNode(const glm::vec3& position, const glm::vec3& velocity){
        particle_state_.positions.push_back(position);
        particle_state_.velocities.push_back(velocity);
        auto particle_node = make_unique<SceneNode>();
        particle_node->GetTransform().SetPosition(position);
        particle_node->CreateComponent<RenderingComponent>(sphere_mesh_);
        particle_node->CreateComponent<ShadingComponent>(shader_);
        this->AddChild(std::move(particle_node));
    };
    void AddSpring(int index1, int index2, float rest_length, float stiffness){
        particle_system_.AddSpring(index1, index2, rest_length, stiffness);
        
    };

    
 protected:
     PendulumSystem particle_system_;
     ParticleState particle_state_;
     std::shared_ptr<PhongShader> shader_;
     std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator_;
     float dt_;
     std::vector<SceneNode*> particles_;
     float time_;
     std::shared_ptr<VertexObject> sphere_mesh_;
     std::shared_ptr<ParticleState> original_state_;
};

}  // namespace GLOO
