#ifndef PATCH_NODE_H_
#define PATCH_NODE_H_

#include <string>
#include <vector>

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"

#include "CurveNode.hpp"

namespace GLOO {
struct PatchPoint {
  glm::vec3 P;
  glm::vec3 N;
};

class PatchNode : public SceneNode {
 public:
  PatchNode(std::vector<glm::vec3> controlPoints, SplineBasis splineBasis);

 private:
  void PlotPatch();
  float b1(float u);
  float b2(float u);
  float b3(float u);
  float b4(float u);
  float db1(float u);
  float db2(float u);
  float db3(float u);
  float db4(float u);

  std::vector<glm::vec3> control_points_;
  std::vector<glm::mat4> Gs_;
  SplineBasis spline_basis_;

  std::shared_ptr<VertexObject> patch_mesh_;
  std::shared_ptr<ShaderProgram> shader_;

  const int N_SUBDIV_ = 50;
};
}  // namespace GLOO

#endif
