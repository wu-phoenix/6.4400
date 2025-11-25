#ifndef GLOO_SIMPLE_SHADER_H_
#define GLOO_SIMPLE_SHADER_H_

#include "ShaderProgram.hpp"

namespace GLOO {
// A simple shader for debug purposes.
class ShadowShader : public ShaderProgram {
 public:
  ShadowShader();
  void SetTargetNode(const SceneNode& node,
                     const glm::mat4& model_matrix) const override;
  void SetCamera(const glm::mat4& world_to_ndc_matrix, const glm::mat4& view_matrix) const;

 private:
  void AssociateVertexArray(VertexArray& vertex_array) const;
};
}  // namespace GLOO

#endif
