#include "CurveNode.hpp"

#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/shaders/SimpleShader.hpp"
#include "gloo/InputManager.hpp"

namespace GLOO {
CurveNode::CurveNode(const std::vector<glm::vec3>& controlPoints, SplineBasis basis)
    : SceneNode() {
  // TODO: this node should represent a single spline curve.
  // Think carefully about what data defines a curve and how you can
  // render it.

  // Initialize the VertexObjects and shaders used to render the control points,
  // the curve, and the tangent line.
  control_points_ = controlPoints;
  // these are the vectors for the coordinates for the control points
  std::cout << "Control points received: \n";
  for (const auto& point : control_points_) {
    std::cout << "  (" << point.x << ", " << point.y << ", " << point.z << ")\n";
  }
  sphere_mesh_ = PrimitiveFactory::CreateSphere(0.015f, 25, 25);
  curve_polyline_ = std::make_shared<VertexObject>();
  tangent_line_ = std::make_shared<VertexObject>();
  shader_ = std::make_shared<PhongShader>();
  polyline_shader_ = std::make_shared<SimpleShader>();
  spline_basis_ = basis;

  std::cout << "flag1\n";

  auto dummy_positions = make_unique<PositionArray>();
  dummy_positions->push_back(glm::vec3(0, 0, 0));
  dummy_positions->push_back(glm::vec3(1, 0, 0));
  auto dummy_indices = make_unique<IndexArray>();
  dummy_indices->push_back(0);
  dummy_indices->push_back(1);
  std::cout << "flag2\n";
  curve_polyline_->UpdatePositions(std::move(dummy_positions));
  curve_polyline_->UpdateIndices(std::move(dummy_indices));
  std::cout << "flag3\n";

  auto dummy_positions2 = make_unique<PositionArray>();
  dummy_positions2->push_back(glm::vec3(0, 0, 0));
  dummy_positions2->push_back(glm::vec3(0, 1, 0));
  auto dummy_indices2 = make_unique<IndexArray>();
  dummy_indices2->push_back(0);
  dummy_indices2->push_back(1);
  tangent_line_->UpdatePositions(std::move(dummy_positions2));
  tangent_line_->UpdateIndices(std::move(dummy_indices2));
  std::cout << "flag4\n";
  bezier_matrix_ = glm::mat4(
    1, -3, 3, -1,
    0, 3, -6, 3,
    0, 0, 3, -3,
    0, 0, 0, 1
  );

  bspline_matrix_ = glm::mat4(
    1.f/6, -3.f/6, 3.f/6, -1.f/6,
    4.f/6, 0, -6.f/6, 3.f/6,
    1.f/6, 3.f/6, 3.f/6, -3.f/6,
    0, 0, 0, 1.f/6
  );
  
  inv_bezier_matrix_ = glm::mat4(
    1, 1, 1, 1,
    0, 1.f/3, 2.f/3, 1,
    0, 0, 1.f/3, 1,
    0, 0, 0, 1
  );

  inv_bspline_matrix_ = glm::mat4(
    1, 1, 1, 1,
    -1, 0, 1, 2,
    2.f/3, -1.f/3, 2.f/3, 11.f/3,
    0, 0, 0, 6
  );
  std::cout << "inverted bspline matrix\n" << inv_bspline_matrix_[0][0] << " " << inv_bspline_matrix_[0][1] << " " << inv_bspline_matrix_[0][2] << " " << inv_bspline_matrix_[0][3] << "\n"
            << inv_bspline_matrix_[1][0] << " " << inv_bspline_matrix_[1][1] << " " << inv_bspline_matrix_[1][2] << " " << inv_bspline_matrix_[1][3] << "\n"
            << inv_bspline_matrix_[2][0] << " " << inv_bspline_matrix_[2][1] << " " << inv_bspline_matrix_[2][2] << " " << inv_bspline_matrix_[2][3] << "\n"
            << inv_bspline_matrix_[3][0] << " " << inv_bspline_matrix_[3][1] << " " << inv_bspline_matrix_[3][2] << " " << inv_bspline_matrix_[3][3] << "\n";; 

  InitCurve();
}

void CurveNode::Update(double delta_time) {

  // Prevent multiple toggle.
  static bool prev_released = true;

  if (can_toggle_ && InputManager::GetInstance().IsKeyPressed('T')) {
    if (prev_released) {
      // TODO: implement toggling spline bases.
      // Control points shouldn't change.
      std::cout << "T pressed\n";
      ToggleSplineBasis();
    }
    prev_released = false;
  } else if (can_toggle_ && InputManager::GetInstance().IsKeyPressed('B')) {
    if (prev_released) {
      // TODO: implement converting conrol point geometry from Bezier to
      // B-Spline basis.
      std::cout << "B pressed\n";
      ConvertToBSpline();
    }
    prev_released = false;
  } else if (can_toggle_ && InputManager::GetInstance().IsKeyPressed('Z')) {
    if (prev_released) {
      // TODO: implement converting conrol point geometry from B-Spline to
      // Bezier basis.
      std::cout << "Z pressed\n";
      ConvertToBezier();
    }
    prev_released = false;
  } else {
    prev_released = true;
  }
  PlotCurve();
  // std::cout << "plotted curve in update\n";
  PlotTangentLine();
  // std::cout << "plotted tangent line in update\n";
  PlotControlPoints();
  // std::cout << "plotted control points in update\n";
  // std::cout << "control point coordinates: " << control_points_[0].x << ", " << control_points_[0].y << ", " << control_points_[0].z << "\n" << control_points_[1].x << ", " << control_points_[1].y << ", " << control_points_[1].z << "\n" << control_points_[2].x << ", " << control_points_[2].y << ", " << control_points_[2].z << "\n" << control_points_[3].x << ", " << control_points_[3].y << ", " << control_points_[3].z << "\n";
}

void CurveNode::ToggleSplineBasis() {
  // TODO: implement toggling between Bezier and B-Spline bases. Control points shouldn't change.
  if (spline_basis_ == SplineBasis::Bezier) {
    spline_basis_ = SplineBasis::BSpline;
  } else {
    spline_basis_ = SplineBasis::Bezier;
  }
  PlotControlPoints();
}

void CurveNode::ConvertGeometry() {
  // TODO: implement converting the control points between bases. The *CURVE* stays the same, but the control points change.

  if (spline_basis_ == SplineBasis::Bezier) {
    ConvertToBSpline();
  } else {
    ConvertToBezier();
  }
}

void CurveNode::ConvertToBSpline() {
  glm::mat3x4 cp = glm::mat3x4(
    control_points_[0][0], control_points_[1][0], control_points_[2][0], control_points_[3][0],  // x-coords
    control_points_[0][1], control_points_[1][1], control_points_[2][1], control_points_[3][1],  // y-coords  
    control_points_[0][2], control_points_[1][2], control_points_[2][2], control_points_[3][2]   // z-coords
  );
  glm::mat3x4 new_cp;
  glm::mat4 conversion_mat =  inv_bspline_matrix_ * bezier_matrix_;
  std::cout << cp[0][0] << " " << cp[0][1] << " " << cp[0][2] << " " << cp[0][3] << "\n"
            << cp[1][0] << " " << cp[1][1] << " " << cp[1][2] << " " << cp[1][3] << "\n"
            << cp[2][0] << " " << cp[2][1] << " " << cp[2][2] << " " << cp[2][3] << "\n";
  std::cout << "conversion matrix: \n" << conversion_mat[0][0] << " " << conversion_mat[0][1] << " " << conversion_mat[0][2] << " " << conversion_mat[0][3] << "\n"
            << conversion_mat[1][0] << " " << conversion_mat[1][1] << " " << conversion_mat[1][2] << " " << conversion_mat[1][3] << "\n"
            << conversion_mat[2][0] << " " << conversion_mat[2][1] << " " << conversion_mat[2][2] << " " << conversion_mat[2][3] << "\n"
            << conversion_mat[3][0] << " " << conversion_mat[3][1] << " " << conversion_mat[3][2] << " " << conversion_mat[3][3] << "\n";
  
    // this multiplication was assuming left to right multiplication, but glm does right to left multiplication
    
    for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      new_cp[i][j] = 0;
      for (int k = 0; k < 4; k++) {
        new_cp[i][j] += cp[i][k] * conversion_mat[k][j];
      }
    }
  }

  std::cout << "new control points: \n" << new_cp[0][0] << " " << new_cp[0][1] << " " << new_cp[0][2] << " " << new_cp[0][3] << "\n"
            << new_cp[1][0] << " " << new_cp[1][1] << " " << new_cp[1][2] << " " << new_cp[1][3] << "\n"
            << new_cp[2][0] << " " << new_cp[2][1] << " " << new_cp[2][2] << " " << new_cp[2][3] << "\n";
  for (int i = 0; i < 4; i++) {
    control_points_[i] = glm::vec3(new_cp[0][i], new_cp[1][i], new_cp[2][i]);
  }
  PlotControlPoints();  
  PlotCurve();
}

void CurveNode::ConvertToBezier() {
  glm::mat3x4 cp = glm::mat3x4(
    control_points_[0][0], control_points_[1][0], control_points_[2][0], control_points_[3][0],  // x-coords
    control_points_[0][1], control_points_[1][1], control_points_[2][1], control_points_[3][1],  // y-coords  
    control_points_[0][2], control_points_[1][2], control_points_[2][2], control_points_[3][2]   // z-coords
  );
  glm::mat3x4 new_cp;
  glm::mat4 conversion_mat = inv_bezier_matrix_ * bspline_matrix_;
  
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      new_cp[i][j] = 0;
      for (int k = 0; k < 4; k++) {
        new_cp[i][j] += cp[i][k] * conversion_mat[k][j];
      }
    }
  }

  for (int i = 0; i < 4; i++) {
    control_points_[i] = glm::vec3(new_cp[0][i], new_cp[1][i], new_cp[2][i]);
  }
  PlotControlPoints();  
  PlotCurve();
}



CurvePoint CurveNode::EvalCurve(float t) {
  // TODO: implement evaluating the spline curve at parameter value t.
  
  glm::vec4 T_vec = glm::vec4(1, t, t*t, t*t*t);
  glm::vec4 T_deriv = glm::vec4(0, 1, 2*t, 3*t*t);

  glm::mat3x4 cp = glm::mat3x4(
    control_points_[0][0], control_points_[1][0], control_points_[2][0], control_points_[3][0],  // x-coords
    control_points_[0][1], control_points_[1][1], control_points_[2][1], control_points_[3][1],  // y-coords  
    control_points_[0][2], control_points_[1][2], control_points_[2][2], control_points_[3][2]   // z-coords
  );

  if (spline_basis_ == SplineBasis::Bezier) {
    glm::vec3 P = T_vec * bezier_matrix_ * cp;
    glm::vec3 T = T_deriv * bezier_matrix_ * cp;
    // normalize T
    T = glm::normalize(T);
    return CurvePoint{P, T};
  } else {
    glm::vec3 P = T_vec * bspline_matrix_ * cp;
    glm::vec3 T = T_deriv * bspline_matrix_ * cp;
    // normalize T
    T = glm::normalize(T);
    return CurvePoint{P, T};
  }

}

void CurveNode::InitCurve() {
    // Create curve node
    std::cout << "Initializing curve with " << control_points_.size() << " control points.\n";
    auto curve_node = make_unique<SceneNode>();
    curve_node->CreateComponent<ShadingComponent>(polyline_shader_); // Use polyline_shader for lines
    std::cout << "Created curve node and added shading component\n";
    auto& curve_rc = curve_node->CreateComponent<RenderingComponent>(curve_polyline_);
    std::cout << "Created rendering component for curve node\n";
    curve_rc.SetDrawMode(DrawMode::Lines); // Important for lines!
    std::cout << "Set draw mode to lines\n";
    curve_node_ = curve_node.get(); // Store raw pointer
    std::cout << "Created rendering component and set draw mode to lines\n";
    AddChild(std::move(curve_node)); // Transfer ownership
    std::cout << "Added curve node to scenenode and set curve_node pointer\n";
    // Create tangent node  
    auto tangent_node = make_unique<SceneNode>();
    tangent_node->CreateComponent<ShadingComponent>(polyline_shader_);
    auto& tangent_rc = tangent_node->CreateComponent<RenderingComponent>(tangent_line_);
    tangent_rc.SetDrawMode(DrawMode::Lines) ;
    tangent_node_ = tangent_node.get();
    AddChild(std::move(tangent_node));
    std::cout << "Added tangent node to scenenode and set tangent_node pointer\n";
    glm::vec3 color = glm::vec3(0.f, 0.f, 1.f);
    auto material = std::make_shared<Material>(color, color, color, 0);


    // Create control point nodes
    for (int i = 0; i < (int)control_points_.size(); i++) {
        auto sphere_node = make_unique<SceneNode>();
        sphere_node->CreateComponent<ShadingComponent>(shader_);
        sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
        sphere_node->CreateComponent<MaterialComponent>(material);
        control_point_nodes_.push_back(sphere_node.get()); // Store raw pointer
        AddChild(std::move(sphere_node)); // Transfer ownership
    }
    std::cout << "Added " << control_point_nodes_.size() << " control point nodes to scenenode and set control_point_nodes_ pointers\n";
}
void CurveNode::PlotCurve() {
  // TODO: plot the curve by updating the positions of its VertexObject.

  auto positions = make_unique<PositionArray>();
  auto indices = make_unique<IndexArray>();
  for (int i = 0; i <= N_SUBDIV_; i++) {
    float t = (float)i / (float)N_SUBDIV_;
    CurvePoint curve_point = EvalCurve(t);
    positions->push_back(curve_point.P);
    indices->push_back(i);
  }

  // std::cout << "Generated " << positions->size() << " curve positions\n";
  // if (!positions->empty()) {
  //   std::cout << "First curve point: (" << (*positions)[0].x << ", " 
  //             << (*positions)[0].y << ", " << (*positions)[0].z << ")\n";
  // }

  curve_polyline_->UpdatePositions(std::move(positions));
  curve_polyline_->UpdateIndices(std::move(indices));

  // std::cout << "Updated curve vertex object\n";
}

// for (int i = 0; i < control_points_.size(); i++) {
  //   auto sphere_node = make_unique<SceneNode>();
  //   sphere_node->GetTransform().SetPosition(control_points_[i]);
  //   sphere_node->CreateComponent<ShadingComponent>(shader_);
  //   sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);
  //   glm::vec3 color;
  //   if (SplineBasis::Bezier == spline_basis_) {
  //     color = glm::vec3(1.f, 0.f, 0.f);
  //   } else {
  //     color = glm::vec3(0.f, 1.f, 0.f);
  //   }
  //   auto material = std::make_shared<Material>(color, color, color, 0);
  //   sphere_node->CreateComponent<MaterialComponent>(material);
    
  // }
void CurveNode::PlotControlPoints() {
  // TODO: plot the curve control points.
  

  // std::cout << "inside plot control points\n";
  // std::cout << "Control points to plot: \n";
  // std::cout << "Number of control points: " << control_point_nodes_.size() << "\n";
  for( int i = 0; i < (int)control_point_nodes_.size(); i++) {
    // std::cout << "  (" << control_points_[i].x << ", " << control_points_[i].y << ", " << control_points_[i].z << ")\n";
    control_point_nodes_[i]->GetTransform().SetPosition(control_points_[i]);
    glm::vec3 color;
    if (SplineBasis::Bezier == spline_basis_) {
      color = glm::vec3(1.f, 0.f, 0.f);
    } else {
      color = glm::vec3(0.f, 1.f, 0.f);
    }
    auto material = std::make_shared<Material>(color, color, color, 0);
    // std::cout << "Setting control point " << i << " to position (" 
    //           << control_points_[i].x << ", " 
    //           << control_points_[i].y << ", " 
    //           << control_points_[i].z << ") with color (" 
    //           << color.x << ", " 
    //           << color.y << ", " 
    //           << color.z << ")\n";
    control_point_nodes_[i]->GetComponentPtr<MaterialComponent>()->SetMaterial(material);
  }
}



void CurveNode::PlotTangentLine() {
  // TODO: implement plotting a line tangent to the curve.
  // Below is a sample implementation for rendering a line segment
  // onto the screen. Note that this is just an example. This code
  // currently has nothing to do with the spline.

  CurvePoint tangent_point = EvalCurve(0.5);

  auto positions = make_unique<PositionArray>();
  positions->push_back(tangent_point.P + tangent_point.T * 0.1f);
  positions->push_back(tangent_point.P - tangent_point.T * 0.1f);

  auto indices = make_unique<IndexArray>();
  indices->push_back(0);
  indices->push_back(1);

  tangent_line_->UpdatePositions(std::move(positions));
  tangent_line_->UpdateIndices(std::move(indices));
}
}  // namespace GLOO
