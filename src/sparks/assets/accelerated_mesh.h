#pragma once
#include "sparks/assets/aabb.h"
#include "sparks/assets/mesh.h"
#include "sparks/acceleration/tree.hpp"
#include "sparks/acceleration/bvh.h"

namespace sparks {

namespace {
struct TreeNode {
  AxisAlignedBoundingBox aabb{};
  int child[2]{-1, -1};
};
}  // namespace

class AcceleratedMesh : public Mesh {
  // Aliases for bounding volume hierarchy
  public:
    using Bvh = BinaryTree<BvhNodeData>;
    using BvhNode = Bvh::Node;
    //AcceleratedMesh() = default;
    explicit AcceleratedMesh(const Mesh &mesh);
    AcceleratedMesh(const std::vector<Vertex> &vertices,
                    const std::vector<uint32_t> &indices);
    float TraceRay(const glm::vec3 &origin,
                    const glm::vec3 &direction,
                    float t_min,
                    HitRecord *hit_record) const override;
    int GetNumFaces();
    void BuildAccelerationStructure(); // build bvh
    Bvh* GetBvh() {
      return bvh_.get();
    }

  private:
    const int LEAF_FACES_ {5}; // The maximum number of faces stored in a bvh leaf
    std::unique_ptr<Bvh > bvh_; // bounding volume hierarchy
    // Length = f, each entry stores (i0,i1,i2), the indices of the three vertices of this face. Necessary?
    //std::vector<glm::ivec3> face2vertex_indices_; 

    // Get face2vertex_indices_, given indices_
    //void GetFaces_();
    /* Split Bvh on a node with a list of faces.This node is created, but the content, children ptrs are all nullptr.
    */ 
    void BuildBvhRecursive_(BvhNode* node, const std::vector<int> & face_indices);

    // Find bounding box for a list of faces
    [[nodiscard]] AxisAlignedBoundingBox FindBox_(const std::vector<int>& face_indices) const;

    // Split the list of faces into two groups according to axis ("x", "y" or "z")
    void SplitFacesAlongAxis_(
      const std::string & axis,
      const std::vector<int>& face_indices,
      std::vector<int>& result_face_indices_left,
      std::vector<int>& result_face_indices_right);

    // Return the center of face, given index
    glm::vec3 GetCenter(int face_index);

   

};
}  // namespace sparks
