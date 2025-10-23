#include "gloo/SceneNode.hpp"
#include "gloo/utils.hpp"
#include "ForwardEulerIntegrator.hpp"
#include "SimpleSystem.hpp"
#include "ParticleState.hpp"
#include "ParticleSystemBase.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "PendulumSystemNode.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "ClothSystemNode.hpp"
#include "gloo/InputManager.hpp"


namespace GLOO {

ClothSystemNode::ClothSystemNode(
        std::unique_ptr<IntegratorBase<PendulumSystem, ParticleState>> integrator,
        float dt, int width, int height)
        : PendulumSystemNode(std::move(integrator), dt) 
        {
    // this will be a shared mesh for all particles (aka the one particle for this system)
    
    // Base class constructor already initialized integrator_, dt_, time_,
    // shader_, and sphere_mesh_. We configure cloth-specific data below.
    width_ = width;
    height_ = height;

    sphere_mesh_ = PrimitiveFactory::CreateSphere(0.1f, 10, 10);
    shader_ = std::make_shared<PhongShader>();  
    particle_state_ = ParticleState();
    particle_system_ = PendulumSystem();
    particle_system_.SetDragCoefficient(0.6f);
    // Create a cloth grid of particles
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            particle_state_.positions.push_back(glm::vec3(j * 0.5f, -i * 0.5f, 0.0f));
            particle_state_.velocities.push_back(glm::vec3(0.0f));
            particle_system_.AddMass(1.0f);
        }
    }

    particle_system_.SetFixedIndices({GetIndex(0, 0, width), GetIndex(0, width - 1, width)});

    // add springs
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int index = GetIndex(i, j, width);
            // structural springs
            if (j < width - 1) { // right neighbor
                int right_index = GetIndex(i, j + 1, width);
                particle_system_.AddSpring(index, right_index, 0.5f, 100.0f);
            }
            if (i < height - 1) { // bottom neighbor
                int bottom_index = GetIndex(i + 1, j, width);
                particle_system_.AddSpring(index, bottom_index, 0.5f, 100.0f);
            }
            // shear springs
            if (i < height - 1 && j < width - 1) { // bottom-right neighbor
                int bottom_right_index = GetIndex(i + 1, j + 1, width);
                particle_system_.AddSpring(index, bottom_right_index, glm::length(glm::vec3(0.5f, -0.5f, 0.0f)), 100.0f);
            }
            if (i < height - 1 && j > 0) { // bottom-left neighbor
                int bottom_left_index = GetIndex(i + 1, j - 1, width);
                particle_system_.AddSpring(index, bottom_left_index, glm::length(glm::vec3(-0.5f, -0.5f, 0.0f)), 100.0f);
            }
            // bend springs
            if (j < width - 2) { // right-right neighbor
                int right_right_index = GetIndex(i, j + 2, width);
                particle_system_.AddSpring(index, right_right_index, 1.0f, 100.0f);
            }
            if (i < height - 2) { // bottom-bottom neighbor
                int bottom_bottom_index = GetIndex(i + 2, j, width);
                particle_system_.AddSpring(index, bottom_bottom_index, 1.0f, 100.0f);
            }
        }
    }


    // create wiremesh of cloth
    cloth_mesh_ = std::make_shared<VertexObject>();
    auto positions = make_unique<PositionArray>();
    auto indices = make_unique<IndexArray>();
    auto normals = make_unique<NormalArray>();
    for (size_t i = 0; i < particle_state_.positions.size(); ++i) {
        positions->push_back(particle_state_.positions[i]);        
    }
    // add triangles
    for (int i = 0; i < height-1; ++i) {
        for (int j = 0; j < width - 1; ++j) {
            int tl = GetIndex(i, j, width);
            int tr = GetIndex(i, j + 1, width);
            int bl = GetIndex(i + 1, j, width);
            int br = GetIndex(i + 1, j + 1, width);

            // add both triangles

            indices->push_back(tl);
            indices->push_back(bl);
            indices->push_back(tr);

            indices->push_back(tr);
            indices->push_back(bl);
            indices->push_back(br); 
            
            // add normals for triangles
            normals->push_back(glm::vec3(0.0f, 0.0f, 1.0f));
            normals->push_back(glm::vec3(0.0f, 0.0f, 1.0f));
        }
    }



    original_state_ = std::make_shared<ParticleState>(particle_state_);


    cloth_mesh_->UpdatePositions(std::move(positions));
    cloth_mesh_->UpdateIndices(std::move(indices));

    this->CreateComponent<RenderingComponent>(cloth_mesh_);
    this->CreateComponent<ShadingComponent>(shader_);


    // for each particle state, create a SceneNode that holds the mesh/visualization.
    // We'll constantly update this with the particle state.
    for (size_t i = 0; i < particle_state_.positions.size(); ++i) {
        auto particle_node = make_unique<SceneNode>();
        particle_node->GetTransform().SetPosition(particle_state_.positions[i]);
        particle_node->CreateComponent<RenderingComponent>(sphere_mesh_);
        particle_node->CreateComponent<ShadingComponent>(shader_);
        // auto temp_pointer = particle_node.get();
        this->AddChild(std::move(particle_node));
    }
}

void ClothSystemNode::Update(double dt) 
     {

        static bool prev_released = true;
        if (InputManager::GetInstance().IsKeyPressed('R')) {
            if (prev_released) {
            particle_state_ = *original_state_;
            }
            prev_released = false;
        } else if (InputManager::GetInstance().IsKeyReleased('R')) {
            prev_released = true;
        }


    // std::cout << "updating pendulum system node" << std::endl;
        particle_state_ = integrator_->Integrate(particle_system_, particle_state_, time_, dt_);
        time_ += dt_;
        // update particle nodes to visualize
        cloth_mesh_->UpdatePositions(make_unique<PositionArray>(particle_state_.positions));
        for (size_t i = 0; i < this->GetChildrenCount(); ++i) {
            this->GetChild(i).GetTransform().SetPosition(particle_state_.positions[i]);
        }

        // update normals for cloth mesh
        auto normals = make_unique<NormalArray>();
        for (int i = 0; i < height_-1; ++i) {
            for (int j = 0; j < width_ - 1; ++j) {
                int tl = GetIndex(i, j, width_);
                int tr = GetIndex(i, j + 1, width_);
                int bl = GetIndex(i + 1, j, width_);
                int br = GetIndex(i + 1, j + 1, width_);
                
                // add normals for triangles
                normals->push_back(glm::cross(
                    particle_state_.positions[bl] - particle_state_.positions[tl],
                    particle_state_.positions[tr] - particle_state_.positions[tl]
                ));
                normals->push_back(glm::cross(
                    particle_state_.positions[br] - particle_state_.positions[tr],
                    particle_state_.positions[bl] - particle_state_.positions[tr]
                ));
            }
        }
        cloth_mesh_->UpdateNormals(std::move(normals));
    }

   


}  // namespace GLOO