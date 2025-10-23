#ifndef FORWARD_RK4_INTEGRATOR_H_
#define FORWARD_RK4_INTEGRATOR_H_

#include "IntegratorBase.hpp"

namespace GLOO {
template <class TSystem, class TState>
class ForwardRK4Integrator : public IntegratorBase<TSystem, TState> {
  TState Integrate(const TSystem& system,
                   const TState& state,
                   float start_time,
                   float dt) const override {
    // TODO: Here we are returning the state at time t (which is NOT what we
    // want). Please replace the line below by the state at time t + dt using
    // forward Euler integration.

    // RK4 algorithm: 
    // k1 = f(t, y)
    // k2 = f(t + dt/2, y + k1*dt/2)
    // k3 = f(t + dt/2, y + k2*dt/2)
    // k4 = f(t + dt, y + k3*dt)
    // next state = y + (k1 + 2*k2 + 2*k3 + k4) * (dt/6)

    TState k1 = system.ComputeTimeDerivative(state, start_time);
    TState k2 = system.ComputeTimeDerivative(state + k1 * (dt / 2.0f), start_time + dt / 2.0f);
    TState k3 = system.ComputeTimeDerivative(state + k2 * (dt / 2.0f), start_time + dt / 2.0f);
    TState k4 = system.ComputeTimeDerivative(state + k3 * dt, start_time + dt);
    TState next_state = state + (k1 + 2.0f * k2 + 2.0f * k3 + k4) * (dt / 6.0f);
    return next_state;

  }
};
}  // namespace GLOO

#endif
