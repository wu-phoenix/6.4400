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

    
 private:
    PendulumSystem particle_system_;
    ParticleState particle_state_;
    std::shared_ptr<PhongShader> shader_;
    std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator_;
    float dt_;
    std::vector<SceneNode*> particles_;
    float time_;
    std::shared_ptr<VertexObject> sphere_mesh_;
};

}  // namespace GLOO
