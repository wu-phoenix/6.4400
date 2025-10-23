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
#include "gloo/InputManager.hpp"


namespace GLOO {

class ClothSystemNode : public PendulumSystemNode {
 public:
    ClothSystemNode(std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator,
                                     float dt, int width, int height);

    void Update(double dt) override;

 private:
    // Reuse members from PendulumSystemNode (particle_system_, particle_state_,
    // integrator_, dt_, shader_, sphere_mesh_, etc.). Only cloth-specific
    // helpers live here.
    int GetIndex(int row, int col, int width) {
      return row * width + col;
    }
    std::shared_ptr<VertexObject> cloth_mesh_;
    int width_;
   int height_;   
   std::shared_ptr<ParticleState> original_state_;
};

}  // namespace GLOO
