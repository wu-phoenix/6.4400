#ifndef INTEGRATOR_FACTORY_H_
#define INTEGRATOR_FACTORY_H_

#include "IntegratorBase.hpp"

#include <stdexcept>

#include "gloo/utils.hpp"

#include "IntegratorType.hpp"

namespace GLOO {
class IntegratorFactory {
 public:
  template <class TSystem, class TState>
  static std::unique_ptr<IntegratorBase<TSystem, TState>> CreateIntegrator(
      IntegratorType type) {
    return nullptr;
  }
};
}  // namespace GLOO

#endif
