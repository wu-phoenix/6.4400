#ifndef CURVE_NODE_H_
#define CURVE_NODE_H_

#include <string>
#include <vector>

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"

namespace GLOO {

enum class SplineBasis { Bezier, BSpline };

struct CurvePoint {
  glm::vec3 P;
  glm::vec3 T;
};

class CurveNode : public SceneNode {
 public:
  CurveNode(const std::vector<glm::vec3>& controlPoints, SplineBasis basis);
  void Update(double delta_time) override;
  bool can_toggle_ = false;
 private:
  void ToggleSplineBasis();
  void ConvertGeometry();
  CurvePoint EvalCurve(float t);
  void InitCurve();
  void PlotCurve();
  void PlotControlPoints();
  void PlotTangentLine();
  void ConvertToBezier();
  void ConvertToBSpline();


  
  SplineBasis spline_basis_;
  glm::mat4 bezier_matrix_;
  glm::mat4 bspline_matrix_;
  glm::mat4 inv_bezier_matrix_;
  glm::mat4 inv_bspline_matrix_;
  std::vector<glm::vec3> control_points_;
  std::shared_ptr<VertexObject> sphere_mesh_;
  std::shared_ptr<VertexObject> curve_polyline_;
  std::shared_ptr<VertexObject> tangent_line_;

  std::shared_ptr<ShaderProgram> shader_;
  std::shared_ptr<ShaderProgram> polyline_shader_;
  SceneNode* curve_node_;
  SceneNode* tangent_node_;
  std::vector<SceneNode*> control_point_nodes_;

  const int N_SUBDIV_ = 100;
};
}  // namespace GLOO

#endif
