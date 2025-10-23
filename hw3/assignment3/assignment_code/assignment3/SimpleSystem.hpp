
#pragma once

#include "ParticleSystemBase.hpp"

namespace GLOO {
class SimpleSystem : public ParticleSystemBase {
 public:
    SimpleSystem(){}
    // Compute the time derivative of the particle state at the given time.
    ParticleState ComputeTimeDerivative(const ParticleState& state,
                                                                            float time) const override {
        ParticleState derivative;

        // Make sure the arrays have same size (1 particle)
        derivative.positions.resize(state.positions.size());
        derivative.velocities.resize(state.velocities.size());

        for (size_t i = 0; i < state.positions.size(); ++i) {
            glm::vec3 pos = state.positions[i];
            derivative.positions[i] = glm::vec3(-pos.y, pos.x, 0.0f);
            derivative.velocities[i] = glm::vec3(0.0f);  // not used in this system
        }
        return derivative;
    }
};
}  // namespace GLOO