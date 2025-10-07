#ifndef DIRECTIONAL_LIGHT_H_
#define DIRECTIONAL_LIGHT_H_

#include "gloo/SceneNode.hpp"

namespace GLOO {
class DirectionalLight : public LightBase {
    public: 
        void CalcDirectionLight();

        void SetDirection(const glm::vec3& direction) {
            direction_ = direction;
        }

        glm::vec3 GetDirection() const {
            return direction_;
        }

        LightType GetType() const override {
            return LightType::Directional;
        }

    private:
    glm::vec3 direction_;
    };
}
#endif
