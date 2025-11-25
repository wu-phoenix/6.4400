
#include <cassert>
#include <iostream>
#include <glad/glad.h>
#include <glm/gtx/string_cast.hpp>

#include "Application.hpp"
#include "Scene.hpp"
#include "utils.hpp"
#include "gl_wrapper/BindGuard.hpp"
#include "shaders/ShaderProgram.hpp"
#include "shaders/ShadowShader.hpp"
#include "gloo/lights/DirectionalLight.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "components/ShadingComponent.hpp"
#include "components/CameraComponent.hpp"
#include "debug/PrimitiveFactory.hpp"
#include "gl_wrapper/Framebuffer.hpp"

namespace {
const size_t kShadowWidth = 4096;
const size_t kShadowHeight = 4096;
const glm::mat4 kLightProjection =
    glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 80.0f);
}  // namespace

namespace GLOO {
Renderer::Renderer(Application& application) : application_(application) {
  UNUSED(application_);
  // TODO: you may want to initialize your framebuffer and texture(s) here.
  shadow_depth_tex_ = make_unique<Texture>();
  shadow_depth_tex_->Reserve(GL_DEPTH_COMPONENT, kShadowWidth, kShadowHeight, GL_DEPTH_COMPONENT, GL_FLOAT);
  shadow_fb_ = make_unique<Framebuffer>();
  plain_texture_shader_ = make_unique<PlainTextureShader>();

  // Do not try to render shadow here — we don't have a Scene yet.

  // To render a quad on in the lower-left of the screen, you can assign texture
  // to quad_ created below and then call quad_->GetVertexArray().Render().
  quad_ = PrimitiveFactory::CreateQuad();

  std::cout << "Renderer initialized." << std::endl;
}

void Renderer::SetRenderingOptions() const {
  GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));

  // Enable depth test.
  GL_CHECK(glEnable(GL_DEPTH_TEST));
  GL_CHECK(glDepthFunc(GL_LEQUAL));

  // Enable blending for multi-pass forward rendering.
  GL_CHECK(glEnable(GL_BLEND));
  GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));
}

void Renderer::Render(const Scene& scene) const {
  SetRenderingOptions();
  std::cout << "Set Rendering Options" << std::endl;
  RenderScene(scene);
  std::cout << "Rendered Scene" << std::endl;
  // TODO: When debugging your shadow map, call DebugShadowMap to render a
  // quad at the bottom left corner to display the shadow map.
  DebugShadowMap();
  std::cout << "Debugged Shadow Map" << std::endl;
}

void Renderer::RenderShadow(const Scene& scene,
                            const LightComponent& light,
                            const glm::mat4& world_to_light_ndc) const {
  // Render the scene depth from the provided light NDC matrix into the
  // shadow framebuffer (depth-only).
  BindGuard fb_guard(shadow_fb_.get());  // RAII: unbinds at end of scope
  shadow_fb_->AssociateTexture(*shadow_depth_tex_, GL_DEPTH_ATTACHMENT);

  // set masks and viewport for shadow map rendering
  GL_CHECK(glViewport(0, 0, kShadowWidth, kShadowHeight));
  GL_CHECK(glDepthMask(GL_TRUE));
  GL_CHECK(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
  GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));

  ShadowShader shadow_shader;

  // Only directional lights cast shadows in our simple implementation.
  auto* lb = light.GetLightPtr();
  if (lb->GetType() != LightType::Directional) {
    // restore viewport and masks
    GL_CHECK(glViewport(0, 0, application_.GetWindowSize().x,
                        application_.GetWindowSize().y));
    return;
  }

  // Render all objects' depth from the provided light's point of view.
  auto rendering_info = RetrieveRenderingInfo(scene);

  for (const auto& pr : rendering_info) {
    auto robj_ptr = pr.first;
    SceneNode& node = *robj_ptr->GetNodePtr();

    BindGuard shader_bg(&shadow_shader);
    shadow_shader.SetTargetNode(node, pr.second);
    shadow_shader.SetLightCamera(world_to_light_ndc);

    robj_ptr->Render();
  }

  // reset size of viewport and masks
  GL_CHECK(glViewport(0, 0, application_.GetWindowSize().x,
                      application_.GetWindowSize().y));
  GL_CHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
}

void Renderer::RecursiveRetrieve(const SceneNode& node,
                                 RenderingInfo& info,
                                 const glm::mat4& model_matrix) {
  // model_matrix is parent to world transformation.
  glm::mat4 new_matrix =
      model_matrix * node.GetTransform().GetLocalToParentMatrix();
  auto robj_ptr = node.GetComponentPtr<RenderingComponent>();
  if (robj_ptr != nullptr && node.IsActive())
    info.emplace_back(robj_ptr, new_matrix);

  size_t child_count = node.GetChildrenCount();
  for (size_t i = 0; i < child_count; i++) {
    RecursiveRetrieve(node.GetChild(i), info, new_matrix);
  }
}

Renderer::RenderingInfo Renderer::RetrieveRenderingInfo(
    const Scene& scene) const {
  RenderingInfo info;
  const SceneNode& root = scene.GetRootNode();
  // Efficient implementation without redundant matrix multiplications.
  RecursiveRetrieve(root, info, glm::mat4(1.0f));
  return info;
}

void Renderer::RenderScene(const Scene& scene) const {
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  const SceneNode& root = scene.GetRootNode();
  auto rendering_info = RetrieveRenderingInfo(scene);
  auto light_ptrs = root.GetComponentPtrsInChildren<LightComponent>();
  if (light_ptrs.size() == 0) {
    // Make sure there are at least 2 passes of we don't forget to set color
    // mask back.
    std::cerr << "No light source in the scene!" << std::endl;
    return;
  }

  CameraComponent* camera = scene.GetActiveCameraPtr();

  {
    // Here we first do a depth pass (note that this has nothing to do with the
    // shadow map). The goal of this depth pass is to exclude pixels that are
    // not really visible from the camera, in later rendering passes. You can
    // safely leave this pass here without understanding/modifying it, for
    // assignment 5. If you are interested in learning more, see
    // https://www.khronos.org/opengl/wiki/Early_Fragment_Test#Optimization

    GL_CHECK(glDepthMask(GL_TRUE));
    bool color_mask = GL_FALSE;
    GL_CHECK(glColorMask(color_mask, color_mask, color_mask, color_mask));

    for (const auto& pr : rendering_info) {
      auto robj_ptr = pr.first;
      SceneNode& node = *robj_ptr->GetNodePtr();
      auto shading_ptr = node.GetComponentPtr<ShadingComponent>();
      if (shading_ptr == nullptr) {
        std::cerr << "Some mesh is not attached with a shader during rendering!"
                  << std::endl;
        continue;
      }
      ShaderProgram* shader = shading_ptr->GetShaderPtr();

      BindGuard shader_bg(shader);

      // Set various uniform variables in the shaders.
      shader->SetTargetNode(node, pr.second);
      shader->SetCamera(*camera);

      robj_ptr->Render();
    }
  }

  // The real shadow map/Phong shading passes.
  for (size_t light_id = 0; light_id < light_ptrs.size(); light_id++) {
    // Render the shadow map for this light if it can cast shadows.
    // This will render to `shadow_fb_` and populate `shadow_depth_tex_`.
    LightComponent& light = *light_ptrs.at(light_id);
      glm::mat4 world_to_light_ndc(1.0f);
      if (light.CanCastShadow()) {
        // Compute a simple orthographic projection * view for the directional
        // light (match what shadow shader expects).
        auto* lb = light.GetLightPtr();
        if (lb->GetType() == LightType::Directional) {
          auto* dlight = static_cast<DirectionalLight*>(lb);
          glm::vec3 light_dir = dlight->GetDirection();
          glm::vec3 center(0.0f);
          glm::vec3 light_pos = -light_dir * 1000.0f + center;
          glm::vec3 up(0.0f, 1.0f, 0.0f);
          glm::mat4 light_view = glm::lookAt(light_pos, center, up);
          world_to_light_ndc = kLightProjection * light_view;
          RenderShadow(scene, light, world_to_light_ndc);
        }
      }


    // rendering scene itself now
    GL_CHECK(glDepthMask(GL_FALSE));
    bool color_mask = GL_TRUE;
    GL_CHECK(glColorMask(color_mask, color_mask, color_mask, color_mask));

    for (const auto& pr : rendering_info) {
      auto robj_ptr = pr.first;
      SceneNode& node = *robj_ptr->GetNodePtr();
      auto shading_ptr = node.GetComponentPtr<ShadingComponent>();
      if (shading_ptr == nullptr) {
        std::cerr << "Some mesh is not attached with a shader during rendering!"
                  << std::endl;
        continue;
      }
      ShaderProgram* shader = shading_ptr->GetShaderPtr();

      BindGuard shader_bg(shader);

      // Set various uniform variables in the shaders.
      shader->SetTargetNode(node, pr.second);
      shader->SetCamera(*camera);

      LightComponent& light = *light_ptrs.at(light_id);
      shader->SetLightSource(light);
      // TODO: pass in the shadow texture to the shader via SetShadowMapping if
      // the light can cast shadow.

      // if (light.CanCastShadow()) {
      //   light.GetNodePtr()->GetTransform().GetLocalToWorldMatrix();
      //   shader->SetShadowMapping(*shadow_depth_tex_, world_to_light_ndc);
      // }

      robj_ptr->Render();
    }
  }

  // Re-enable writing to depth buffer.
  GL_CHECK(glDepthMask(GL_TRUE));
}

void Renderer::RenderTexturedQuad(const Texture& texture, bool is_depth) const {
  std::cout << "Rendering textured quad." << std::endl;
  BindGuard shader_bg(plain_texture_shader_.get());
  plain_texture_shader_->SetVertexObject(*quad_);
  plain_texture_shader_->SetTexture(texture, is_depth);
  quad_->GetVertexArray().Render();
}

void Renderer::DebugShadowMap() const {
  GL_CHECK(glDisable(GL_DEPTH_TEST));
  GL_CHECK(glDisable(GL_BLEND));

  glm::ivec2 window_size = application_.GetWindowSize();
  glViewport(0, 0, window_size.x / 4, window_size.y / 4);
  std::cout << "start rendering shadow map debug quad." << std::endl;
  RenderTexturedQuad(*shadow_depth_tex_, true);
  std::cout << "Rendered shadow map debug quad." << std::endl;
  glViewport(0, 0, window_size.x, window_size.y);
}
}  // namespace GLOO
