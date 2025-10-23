
#pragma once

#include "ParticleSystemBase.hpp"

namespace GLOO {
class PendulumSystem : public ParticleSystemBase {
 public:
    PendulumSystem(){}
    // Compute the time derivative of the particle state at the given time.
    ParticleState ComputeTimeDerivative(const ParticleState& state,
                                                                            float time) const override {
        ParticleState derivative;

        // Make sure the arrays have same size
        derivative.positions.resize(state.positions.size());
        derivative.velocities.resize(state.velocities.size());

        // for each particle, iterate through velocities and update position
        for (size_t i = 0; i < state.positions.size(); ++i) {
            glm::vec3 pos = state.positions[i];
            glm::vec3 vel = state.velocities[i];
            // velocity is derivative of position
            derivative.positions[i] = vel;
            // acceleration is derivative of velocity
            glm::vec3 gravity_force = glm::vec3(0.0f, -gravity_, 0.0f);
            glm::vec3 drag_force = -drag_coefficient_ * vel;
            glm::vec3 total_acceleration = (gravity_force + drag_force) / masses_[i]; 
            derivative.velocities[i] = total_acceleration;
        }

        // now handle springs
        for (const auto& spring : springs_) {
            int index1 = static_cast<int>(spring.x);
            int index2 = static_cast<int>(spring.y);
            float rest_length = spring.z;
            float stiffness = spring.w;

            glm::vec3 pos1 = state.positions[index1];
            glm::vec3 pos2 = state.positions[index2];
            glm::vec3 delta = pos2 - pos1;
            float current_length = glm::length(delta);
            glm::vec3 direction = glm::normalize(delta);

            // Hooke's law: F = -k * x
            glm::vec3 spring_force = -stiffness * (current_length - rest_length) * direction;

            // Apply spring force to the accelerations of the two particles
            derivative.velocities[index1] -= spring_force / masses_[index1];
            derivative.velocities[index2] += spring_force / masses_[index2];
        }

        // Handle fixed indices by zeroing out their derivatives
        for (int fixed_index : fixed_indices_) {
            derivative.positions[fixed_index] = glm::vec3(0.0f);
            derivative.velocities[fixed_index] = glm::vec3(0.0f);
        }

        return derivative;
    }

    void SetGravity(float gravity) { gravity_ = gravity; }
    void SetDragCoefficient(float drag_coefficient) { drag_coefficient_ = drag_coefficient; }
    void SetFixedIndices(const std::vector<int>& fixed_indices) { fixed_indices_ = fixed_indices; }
    void SetMasses(const std::vector<float>& masses) { masses_ = masses; }
    void SetSprings(const std::vector<glm::vec4>& springs) { springs_ = springs; }
    void AddSpring(int index1, int index2, float rest_length, float stiffness) {
        springs_.push_back(glm::vec4(index1, index2, rest_length, stiffness));
    }
    void AddMass(float mass) {
        masses_.push_back(mass);
    }
    private:
    float gravity_ = 9.81f;
    float drag_coefficient_ = 0.1f;
    std::vector<int> fixed_indices_;
    std::vector<float> masses_;
    std::vector<glm::vec4> springs_;  // x: index1, y: index2, z: rest_length, w: stiffness
};
}  // namespace GLOO