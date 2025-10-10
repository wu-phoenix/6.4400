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
  }


void SkeletonNode::Update(double delta_time) {
  // Prevent multiple toggle.
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
  std::shared_ptr<VertexObject> vtx_obj =
      MeshLoader::Import(filename).vertex_obj;
  // TODO: store the bind pose mesh in your preferred way.

}

void SkeletonNode::LoadAttachmentWeights(const std::string& path) {
  // TODO: load attachment weights.
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
