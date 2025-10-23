#include "SkeletonNode.hpp"

#include "gloo/utils.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/MeshLoader.hpp"
#include <iostream>
#include <fstream>
#include "gloo/debug/PrimitiveFactory.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/SceneNode.hpp"


namespace GLOO {
SkeletonNode::SkeletonNode(const std::string& filename)
    : SceneNode(), draw_mode_(DrawMode::Skeleton) {
  
  shader_ = std::make_shared<PhongShader>();
  sphere_mesh_ = PrimitiveFactory::CreateSphere(0.03f, 10, 10);
  cylinder_mesh_ = PrimitiveFactory::CreateCylinder(0.02f, 0.1f, 10);
  // skin_mesh_ = 
  // Load all files.

  if (!sphere_mesh_ || !cylinder_mesh_) {
  std::cerr << "FATAL: Failed to create primitive meshes!" << std::endl;
  throw std::runtime_error("Primitive creation failed");
}

  std::cout << "Loading files with prefix: " << filename << std::endl;


  LoadAllFiles(filename);
  std::cout << "Finished loading files." << std::endl;
  DecorateTree();
  std::cout << "Finished decorating tree." << std::endl;
  // Force initial update.
  OnJointChanged();
  std::cout << "Finished initial update." << std::endl;
}

void SkeletonNode::ToggleDrawMode() {
  draw_mode_ =
      draw_mode_ == DrawMode::Skeleton ? DrawMode::SSD : DrawMode::Skeleton;
  // TODO: implement here toggling between skeleton mode and SSD mode.
  // The current mode is draw_mode_;
  // Hint: you may find SceneNode::SetActive convenient here as
  // inactive nodes will not be picked up by the renderer.
  
  if (draw_mode_ == DrawMode::SSD) {
    // ssd_nodes_ptrs_[0]->SetActive(true);
    ssd_mesh_node_->SetActive(true);
    for (auto* sphere : sphere_nodes_ptrs_) {
      sphere->SetActive(false);
    }
    for (auto* cylinder : cylinder_nodes_ptrs_) {
      cylinder->SetActive(false);
    }
  } else {
    // ssd_nodes_ptrs_[0]->SetActive(false);
    ssd_mesh_node_->SetActive(false);
    for (auto* sphere : sphere_nodes_ptrs_) {
      sphere->SetActive(true);
    }
    for (auto* cylinder : cylinder_nodes_ptrs_) {
      cylinder->SetActive(true);
    }
  }
}

void SkeletonNode::DecorateTree() {
  // TODO: set up addtional nodes, add necessary components here.
  // You should create one set of nodes/components for skeleton mode
  // (spheres for joints and cylinders for bones), and another set for
  // SSD mode (you could just use a single node with a RenderingComponent
  // that is linked to a VertexObject with the mesh information. Then you
  // only need to update the VertexObject - updating vertex positions and
  // recalculating the normals, etc.).
  //
  // The code snippet below shows how to add a sphere node to a joint.
  // Suppose you have created member variables shader_ of type
  // std::shared_ptr<PhongShader>, and sphere_mesh_ of type
  // std::shared_ptr<VertexObject>.
  // Here sphere_nodes_ptrs_ is a std::vector<SceneNode*> that stores the
  // pointer so the sphere nodes can be accessed later to change their
  // positions. joint_ptr is assumed to be one of the joint node you created
  // from LoadSkeletonFile (e.g. you've stored a std::vector<SceneNode*> of
  // joint nodes as a member variable and joint_ptr is one of the elements).
  //

  //make all the spheres
  for (std::size_t i = 0; i < joint_nodes_ptrs_.size(); i++) {
    auto joint_ptr = joint_nodes_ptrs_[i];

    auto sphere_node = make_unique<SceneNode>();
    sphere_node->CreateComponent<ShadingComponent>(shader_);
    sphere_node->CreateComponent<RenderingComponent>(sphere_mesh_);

    auto* sphere_ptr = sphere_node.get();
    joint_ptr->AddChild(std::move(sphere_node));
    sphere_nodes_ptrs_.push_back(sphere_ptr); 
  }

  //make all the cylinders
  for (std::size_t i = 1; i < joint_nodes_ptrs_.size(); i++) {
  //root doesn't need a cylinder since cylinders start at "child" joint
    auto joint_ptr = joint_nodes_ptrs_[i];

    auto cylinder_node = make_unique<SceneNode>();
    cylinder_node->CreateComponent<ShadingComponent>(shader_);
    cylinder_node->CreateComponent<RenderingComponent>(cylinder_mesh_);
    auto* cylinder_ptr = cylinder_node.get();
    //add cylinder as child to the joint
    joint_ptr->AddChild(std::move(cylinder_node));
    cylinder_nodes_ptrs_.push_back(cylinder_ptr);
  }

  //make cylinders children of parent joints instead of child joints
//   for (std::size_t i = 0; i < joint_nodes_ptrs_.size(); i++) {
//     auto joint_ptr = joint_nodes_ptrs_[i];

//     for (std::size_t j = 0; j < joint_ptr->GetChildrenCount(); j++) {
//       auto child_joint_ptr = joint_ptr->GetChild(j);
//       auto cylinder_node = make_unique<SceneNode>();
//       cylinder_node->CreateComponent<ShadingComponent>(shader_);
//       cylinder_node->CreateComponent<RenderingComponent>(cylinder_mesh_);
//       auto* cylinder_ptr = cylinder_node.get();
//       //add cylinder as child to the joint
//       joint_ptr->AddChild(std::move(cylinder_node));
//       cylinder_nodes_ptrs_.push_back(cylinder_ptr);
//     }
  
// SSD objects
// use a scene node to hold transform from bind pose to world, just access joint node transform for world to joint

  // Setup SSD nodes - one per joint

  // SSD setup - one helper node per joint to store inverse bind pose
  for (std::size_t i = 0; i < joint_nodes_ptrs_.size(); i++) {
    auto joint_ptr = joint_nodes_ptrs_[i];

    // Set ssd_node transform to be inverse of bind pose of joint
    inverse_bind_matrices_.push_back(glm::inverse(joint_ptr->GetTransform().GetLocalToWorldMatrix()));
  
  }

  auto ssd_node = make_unique<SceneNode>();
ssd_node->CreateComponent<ShadingComponent>(shader_);
ssd_node->CreateComponent<RenderingComponent>(ssd_vertex_obj_);
ssd_node->SetActive(false); // Start in skeleton mode.

// Get pointer before moving
ssd_mesh_node_ = ssd_node.get();

// Add it as a child of THIS SkeletonNode, NOT a joint.
this->AddChild(std::move(ssd_node));
  //temporarry dummy normals
  ssd_vertex_obj_->UpdateNormals(make_unique<NormalArray>(std::vector<glm::vec3>(ssd_vertex_obj_->GetPositions().size(), glm::vec3(0,1,0))));

  // auto ssd_node_root = joint_nodes_ptrs_[0]->GetChild(joint_nodes_ptrs_[0]->GetChildrenCount() - 1);
  // ssd_node_root.CreateComponent<ShadingComponent>(shader_);
  // ssd_node_root.CreateComponent<RenderingComponent>(ssd_vertex_obj_);
  
  // Start with SSD hidden (skeleton mode is default)
  // ssd_mesh_node_->SetActive(false);
  

}


void SkeletonNode::Update(double delta_time) {
  // Prevent multiple toggle.
  std::cout << "SkeletonNode::Update called." << std::endl;
  static bool prev_released = true;
  if (InputManager::GetInstance().IsKeyPressed('S')) {
    if (prev_released) {
      ToggleDrawMode();
    }
    prev_released = false;
  } else if (InputManager::GetInstance().IsKeyReleased('S')) {
    prev_released = true;
  }
}

void SkeletonNode::OnJointChanged() {
  // TODO: this method is called whenever the values of UI sliders change.
  // The new Euler angles (represented as EulerAngle struct) can be retrieved
  // from linked_angles_ (a std::vector of EulerAngle*).
  // The indices of linked_angles_ align with the order of the joints in .skel
  // files. For instance, *linked_angles_[0] corresponds to the first line of
  // the .skel file.
  // if (linked_angles_.empty()) {
  //   std::cout << "Warning: OnJointChanged called but no angles linked yet." << std::endl;
  //   return;
  // }
  std::cout << "OnJointChanged called with " << linked_angles_.size() << " angles." << std::endl;
  for (std::size_t i = 0; i < linked_angles_.size(); i++) {
    auto angles = linked_angles_[i];
    auto joint_ptr = joint_nodes_ptrs_[i];
    Transform& transform = joint_ptr->GetTransform();
    // Convert Euler angles (in degrees) to a quaternion
    transform.SetRotation(glm::quat(glm::vec3(angles->rx, angles->ry, angles->rz))); // Assuming angles are in radians
  }

  // update the cylinders to connect the joints
  for (std::size_t i = 1; i < joint_nodes_ptrs_.size(); i++) {
    auto joint_ptr = joint_nodes_ptrs_[i];
    auto cylinder_ptr = cylinder_nodes_ptrs_[i - 1]; // because cylinders start from the second joint

    // glm::vec3 parent_joint_pos = -glm::vec3(joint_ptr->GetTransform().GetLocalToParentMatrix() * glm::vec4(0,0,0,1)) ;
    // std::cout << "index node " << i << " Parent joint position: (" << parent_joint_pos.x << ", " << parent_joint_pos.y << ", " << parent_joint_pos.z << ")" << std::endl;
    // Compute the rotation to align the cylinder with the vector from parent to child joint

  glm::vec3 to_parent = -joint_ptr->GetTransform().GetPosition();

// The joint's rotation in its local-to-parent matrix rotates this vector
// We want the inverse to get it in the joint's rotated local frame
    glm::mat4 parent_to_local = glm::inverse(joint_ptr->GetTransform().GetLocalToParentMatrix());
    glm::vec3 parent_joint_pos = glm::vec3(parent_to_local * glm::vec4(to_parent, 0.0f));

    float angle = glm::acos(glm::dot(glm::normalize(parent_joint_pos), glm::vec3(0,1,0)));
    glm::vec3 axis = glm::cross(glm::vec3(0,1,0), glm::normalize(parent_joint_pos));
    if (glm::length(axis) < 1e-6) {
      axis = glm::vec3(1,0,0); // arbitrary axis if the vectors are parallel
    } else {
      axis = glm::normalize(axis);
    }
    
    cylinder_ptr->GetTransform().SetScale(glm::vec3(1.0f, 10 * glm::length(parent_joint_pos), 1.0f));
    cylinder_ptr->GetTransform().SetRotation(axis, angle);
  }

// SSD mesh update positions

// auto positions = bind_pose_positions_; // start from bind pose

// for (std::size_t v = 0; v < positions.size(); v++) {
//     glm::vec4 new_pos(0.0f);
//     for (std::size_t j = 0; j < joint_nodes_ptrs_.size(); j++) {
//         float weight = weights_per_vertex_[v][j];
//         if (weight <= 0.0f) continue;

//         auto joint_ptr = joint_nodes_ptrs_[j];
//         glm::mat4 Mj = joint_ptr->GetTransform().GetLocalToWorldMatrix();
//         glm::mat4 BjInv = inverse_bind_matrices_[j];

//         new_pos += weight * (Mj * BjInv * glm::vec4(bind_pose_positions_[v], 1.0f));
//     }
//     positions[v] = glm::vec3(new_pos);
// }

// ssd_vertex_obj_->UpdatePositions(make_unique<PositionArray>(positions));

auto positions = bind_pose_positions_; // Good, start from original mesh

for (std::size_t v = 0; v < positions.size(); v++) {
    glm::vec4 new_pos(0.0f);
    float total_weight = 0.0f;
    for (std::size_t j = 0; j < joint_nodes_ptrs_.size(); j++) {
        float weight = weights_per_vertex_[v][j];
        total_weight += weight;
        if (weight <= 0.001f) continue; // Use a small epsilon

        // Get the joint's CURRENT world matrix
        glm::mat4 Mj = joint_nodes_ptrs_[j]->GetTransform().GetLocalToWorldMatrix();

        // Get the INVERSE BIND matrix from our clean, decoupled array
        glm::mat4 BjInv = inverse_bind_matrices_[j];

        // Apply the standard skinning formula
        glm::vec4 initial_pos = glm::vec4(bind_pose_positions_[v], 1.0f);
        new_pos += weight * (Mj * BjInv * initial_pos);
    }

    if (total_weight > 0.0f) {
        positions[v] = glm::vec3(new_pos / total_weight);
    } else {
        // This is a truly unweighted vertex. Don't leave it at the origin.
        // Place it back at its original bind pose position to prevent spikes.
        positions[v] = bind_pose_positions_[v];
    }

    positions[v] = glm::vec3(new_pos);
}

ssd_vertex_obj_->UpdatePositions(make_unique<PositionArray>(positions));


// SSD update normals
// Very basic normal recalculation by averaging face normals

// // Get deformed vertex positions from SSD
std::vector<glm::vec3> deformed_positions = positions; // copy for modification

std::vector<glm::vec3> normals(deformed_positions.size(), glm::vec3(0.0f));
auto& indices = ssd_vertex_obj_->GetIndices();

for (size_t i = 0; i < indices.size(); i += 3) {
    auto i0 = indices[i], i1 = indices[i+1], i2 = indices[i+2];
    glm::vec3 v0 = deformed_positions[i0];
    glm::vec3 v1 = deformed_positions[i1];
    glm::vec3 v2 = deformed_positions[i2];

    glm::vec3 tri_normal = glm::cross(v1 - v0, v2 - v0);
    normals[i0] += tri_normal;
    normals[i1] += tri_normal;
    normals[i2] += tri_normal;
}

for (auto& n : normals) n = glm::normalize(n);

ssd_vertex_obj_->UpdateNormals(make_unique<NormalArray>(normals));


  std::cout << "Updated " << linked_angles_.size() << " joints." << std::endl;
}

void SkeletonNode::LinkRotationControl(const std::vector<EulerAngle*>& angles) {
  linked_angles_ = angles;
}

void SkeletonNode::LoadSkeletonFile(const std::string& path) {
  // TODO: load skeleton file and build the tree of joints.
    //add gdb breakpoints here to debug file reading
    
    std::cout << "Loading skeleton file: " << path << std::endl;

    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << path << std::endl;
        return;
    }
    std::vector<std::string> lines;  // Store all lines here
    std::string line;
    // Read file line by line
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    std::cout << "Read " << lines.size() << " lines from " << path << std::endl;
    for (std::size_t i = 0; i < lines.size(); i++) {
      // string of the form "x y z parent_index"
      std::istringstream iss(lines[i]);
      float x, y, z;
      int parent_index;
      if (!(iss >> x >> y >> z >> parent_index)) {
          std::cerr << "Error: Invalid line format in " << path << std::endl;
          continue;
      }

      // std::cout << "Joint " << i << ": Position(" << x << ", " << y << ", " << z 
      //           << "), Parent Index: " << parent_index << std::endl;
      // std::cout << "current joint_nodes array" << joint_nodes_ptrs_.size() << std::endl;
      // Create a new joint node and set its properties
      auto joint_node = make_unique<SceneNode>();
      Transform& transform = joint_node->GetTransform();
      transform.SetPosition(glm::vec3(x, y, z));
      transform.SetRotation(glm::quat(1, 0, 0, 0)); // No rotation
      transform.SetScale(glm::vec3(1, 1, 1)); // Uniform
      
      if (parent_index == -1) {
        // This is the root joint
        joint_nodes_ptrs_.push_back(joint_node.get());
        AddChild(std::move(joint_node));
        
      } else if (parent_index >= 0) {
        // Valid parent index, add as child to the parent joint
        
        joint_nodes_ptrs_.push_back(joint_node.get());
        joint_nodes_ptrs_[parent_index]->AddChild(std::move(joint_node));
      } else
      {
        /* code */
        std::cout << "Error: Invalid parent index in " << path << std::endl;
      }
      
    }
  std::cout << "Loaded " << joint_nodes_ptrs_.size() << " joints." << std::endl;
}

void SkeletonNode::LoadMeshFile(const std::string& filename) {
  ssd_vertex_obj_ = std::move(MeshLoader::Import(filename).vertex_obj);
  // TODO: store the bind pose mesh in your preferred way.
  std::cout << "Loading mesh file: " << filename << std::endl;
  bind_pose_positions_ = ssd_vertex_obj_->GetPositions();  // store bind pose
  std::cout << "Loaded mesh with " << bind_pose_positions_.size() << " vertices." << std::endl;
}

void SkeletonNode::LoadAttachmentWeights(const std::string& path) {
  // TODO: load attachment weights.
  std::cout << "Loading attachment weights from: " << path << std::endl;

  std::ifstream file(path);
  if (!file.is_open()) {
      std::cerr << "Error: Could not open file " << path << std::endl;
      return;
  }
  std::string line;
    // Store weights for each vertex
  // Read file line by line
  while (std::getline(file, line)) {
      std::istringstream iss(line);
      std::vector<float> vertex_weights;
      vertex_weights.push_back(0.0f); // Initialize weights for all joints to 0.0f
      float weight;
      // Read weights for the current vertex
      while (iss >> weight) {
          vertex_weights.push_back(weight);
      }    
      weights_per_vertex_.push_back(vertex_weights);
  }
  //print out the weights for debugging
  // for (std::size_t i = 0; i < weights_per_vertex_.size(); ++i) {
  //     std::cout << "Vertex " << i << ": ";
  //     for (float w : weights_per_vertex_[i]) {
  //         std::cout << w << " ";
  //     }
  //     std::cout << std::endl;
  // }

}

void SkeletonNode::LoadAllFiles(const std::string& prefix) {
  std::string prefix_full = GetAssetDir() + prefix;
  // std::cout << "Loading files with prefix: " << prefix_full << std::endl;
  LoadSkeletonFile(prefix_full + ".skel");
  // std::cout << "Finished loading skeleton." << std::endl;
  LoadMeshFile(prefix + ".obj");
  // std::cout << "Finished loading mesh." << std::endl;
  LoadAttachmentWeights(prefix_full + ".attach");
  std::cout << "Finished LoadAllFiles." << std::endl;
}
}  // namespace GLOO
