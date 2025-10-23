#ifndef FORWARD_TRAPEZOID_INTEGRATOR_H_
#define FORWARD_TRAPEZOID_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class ForwardTrapezoidIntegrator : public IntegratorBase<TSystem, TState> {
  TState Integrate(const TSystem& system,
                   const TState& state,
                   float start_time,
                   float dt) const override {
    // TODO: Here we are returning the state at time t (which is NOT what we
    // want). Please replace the line below by the state at time t + dt using
    // forward Euler integration.

    // TState dstate = system.ComputeTimeDerivative(state, start_time) * dt;
    // state = state + dstate;
    // return state;

    TState k1 = system.ComputeTimeDerivative(state, start_time);
    TState predicted_state = state + k1 * dt;
    TState k2 = system.ComputeTimeDerivative(predicted_state, start_time + dt);
    TState next_state = state + (k1 + k2) * (dt / 2.0f);
    return next_state;
  }
};
}  // namespace GLOO

#endif
