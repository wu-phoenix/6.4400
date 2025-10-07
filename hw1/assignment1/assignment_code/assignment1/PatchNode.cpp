#include "PatchNode.hpp"

#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"

namespace GLOO {
PatchNode::PatchNode(std::vector<glm::vec3> controlPoints, SplineBasis splineBasis) {
  spline_basis_ = splineBasis;
  shader_ = std::make_shared<PhongShader>();
  patch_mesh_ = std::make_shared<VertexObject>();
  auto dummy_positions = make_unique<PositionArray>();
  dummy_positions->push_back(glm::vec3(0, 0, 0));
  dummy_positions->push_back(glm::vec3(1, 0, 0));
  auto dummy_indices = make_unique<IndexArray>();
  dummy_indices->push_back(0);
  dummy_indices->push_back(1);

  patch_mesh_->UpdatePositions(std::move(dummy_positions));
  patch_mesh_->UpdateIndices(std::move(dummy_indices));

  control_points_ = controlPoints;
  if (control_points_.size() != 16) {
    throw std::runtime_error("PatchNode needs exactly 16 control points!");
  }

  // Set up G matrices for the patch.
  for (int i = 0; i < 4; i++) {
    glm::mat4 G;
    for (int j = 0; j < 4; j++) {
      G[0][j] = control_points_[i * 4 + j].x;
      G[1][j] = control_points_[i * 4 + j].y;
      G[2][j] = control_points_[i * 4 + j].z;
      // G[3][j] = 1.0f;
    }
    Gs_.push_back(G);
  }

  // TODO: this node should represent a single tensor product patch.
  // Think carefully about what data defines a patch and how you can
  // render it.
  glm::vec3 color = glm::vec3(1.f, 0.f, 0.f);
  auto material = std::make_shared<Material>(color, color, color, 0);
  material->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
  material->SetSpecularColor(glm::vec3(0.5f, 0.5f, 0.5f));
  material->SetShininess(32.0f);
  CreateComponent<MaterialComponent>(material);
  CreateComponent<ShadingComponent>(shader_);

  CreateComponent<RenderingComponent>(patch_mesh_);
  RenderingComponent* rendering_component =
  GetComponentPtr<RenderingComponent>();

// 3. Tell it to render triangles
  rendering_component->SetDrawMode(DrawMode::Triangles);

  PlotPatch();
}


void PatchNode::PlotPatch() {
  
  auto positions = make_unique<PositionArray>();
  auto normals = make_unique<NormalArray>();
  auto indices = make_unique<IndexArray>();

// TODO: fill "positions", "normals", and "indices"

// B1 = (1-u)^3
// B2 = 3u(1-u)^2
// B3 = 3u^2(1-u)
// B4 = u^3
float (*b)(float, int);
float (*db)(float, int);

if (spline_basis_ == SplineBasis::BSpline) {
  b = [](float t, int i) {
    switch(i) {
      case 0: return ((1 - t) * (1 - t) * (1 - t)) / 6.0f;
      case 1: return (3*t*t*t - 6*t*t + 4) / 6.0f;
      case 2: return (-3*t*t*t + 3*t*t + 3*t + 1) / 6.0f;
      case 3: return (t*t*t) / 6.0f;
    }
    return 0.f;
  };
  db = [](float t, int i) {
    switch(i) {
      case 0: return -0.5f * (1 - t) * (1 - t);
      case 1: return (9*t*t - 12*t) / 6.0f;
      case 2: return (-9*t*t + 6*t + 3) / 6.0f;
      case 3: return 0.5f * t * t;
    }
    return 0.f;
  };
} else if (spline_basis_ == SplineBasis::Bezier) {
  b = [](float t, int i) {
    switch(i) {
      case 0: return (1-t)*(1-t)*(1-t);
      case 1: return 3*t*(1-t)*(1-t);
      case 2: return 3*t*t*(1-t);
      case 3: return t*t*t;
    }
    return 0.f;
  };
  db = [](float t, int i) {
    switch(i) {
      case 0: return -3*(1-t)*(1-t);
      case 1: return 3*(1-t)*(1-3*t);
      case 2: return 3*t*(2-3*t);
      case 3: return 3*t*t;
    }
    return 0.f;
  };
}
 for (int i = 0; i <= N_SUBDIV_; i++) {
    float u = (float)i / N_SUBDIV_;

    for (int j = 0; j <= N_SUBDIV_; j++) {
      float v = (float)j / N_SUBDIV_;

      // Compute position explicitly
      glm::vec3 P(0.f);
      glm::vec3 du(0.f);
      glm::vec3 dv(0.f);

      for (int ii = 0; ii < 4; ii++) {
        float Bu = b(u, ii);
        float dBu = db(u, ii);
        for (int jj = 0; jj < 4; jj++) {
          float Bv = b(v, jj);
          float dBv = db(v, jj);

          // index in flattened control_points_
          int idx = ii*4 + jj;
          glm::vec3 cp = control_points_[idx];

          // Position
          P += cp * (Bu * Bv);

          // Tangents
          du += cp * (dBu * Bv);
          dv += cp * (Bu * dBv);
        }
      }

      positions->push_back(P);
      normals->push_back(glm::normalize(glm::cross(du, dv)));
    }
  }

for (int i = 0; i < N_SUBDIV_; i++) {
    for (int j = 0; j < N_SUBDIV_; j++) {
      int row1 = i * (N_SUBDIV_ + 1);
      int row2 = (i + 1) * (N_SUBDIV_ + 1);

      // First triangle
      indices->push_back(row1 + j);
      indices->push_back(row1 + j + 1);
      indices->push_back(row2 + j);

      // Second triangle
      indices->push_back(row2 + j);
      indices->push_back(row1 + j + 1);
      indices->push_back(row2 + j + 1);
    }
  }

  patch_mesh_->UpdatePositions(std::move(positions));
  patch_mesh_->UpdateNormals(std::move(normals));
  patch_mesh_->UpdateIndices(std::move(indices));
}


}  // namespace GLOO
