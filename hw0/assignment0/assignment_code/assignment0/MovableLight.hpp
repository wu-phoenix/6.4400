#ifndef MOVABLE_LIGHT_H_
#define MOVABLE_LIGHT_H_

#include "gloo/SceneNode.hpp"

namespace GLOO {
class MovableLight : public SceneNode {
    public: 
        MovableLight();
        void Update(double delta_time) override;
};
}
#endif
