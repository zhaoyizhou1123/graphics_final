#include "sparks/assets/accelerated_mesh.h"

#include "algorithm"
#include <numeric>

namespace sparks {
AcceleratedMesh::AcceleratedMesh(const Mesh &mesh) : Mesh(mesh) {
  BuildAccelerationStructure();
}

AcceleratedMesh::AcceleratedMesh(const std::vector<Vertex> &vertices,
                                 const std::vector<uint32_t> &indices)
    : Mesh(vertices, indices) {
  BuildAccelerationStructure();
}

float AcceleratedMesh::TraceRay(const glm::vec3 &origin,
                                const glm::vec3 &direction,
                                float t_min,
                                HitRecord *hit_record) const {
  return Mesh::TraceRay(origin, direction, t_min, hit_record);
}

int AcceleratedMesh::GetNumFaces()
{
  return indices_.size() / 3;
}

void AcceleratedMesh::BuildAccelerationStructure() {
  int num_faces = GetNumFaces();
  std::vector<int> faces(num_faces);
  std::iota(faces.begin(), faces.end(), 0); // Fill with 0, 1, 2, ...
  // Initialize bvh_
  bvh_ = std::make_unique<Bvh>();
  BvhNode* root = bvh_->GetRoot();
  BuildBvhRecursive_(root, faces);
  LAND_INFO("Building bounding volume hierarchy completed");
}

//void AcceleratedMesh::GetFaces_()
//{
//  if (indices_.size() % 3 != 0) {
//    LAND_ERROR("Indices size ({}) not a multiple of 3!", indices_.size());
//  }
//  int num_faces = indices_.size();
//  face2vertex_indices_ = std::vector<glm::ivec3>(num_faces);
//  for (int f = 0; f < num_faces; f++) {
//    face2vertex_indices_[f] = glm::ivec3{ indices_[3 * f], indices_[3 * f + 1], indices_[3 * f + 2] };
//  }
//}

AxisAlignedBoundingBox AcceleratedMesh::FindBox_(const std::vector<int>& face_indices) const
{
  const Vertex& v0 = vertices_[indices_[0]];
  const Vertex& v1 = vertices_[indices_[1]];
  const Vertex& v2 = vertices_[indices_[2]];
  const AxisAlignedBoundingBox& box0 = AxisAlignedBoundingBox(v0.position);
  const AxisAlignedBoundingBox& box1 = AxisAlignedBoundingBox(v1.position);
  const AxisAlignedBoundingBox& box2 = AxisAlignedBoundingBox(v2.position);
  AxisAlignedBoundingBox result_box = box0 | box1 | box2;
  for (int f = 1; f < face_indices.size(); f++) {
    const Vertex& v0_temp = vertices_[indices_[3*f]];
    const Vertex& v1_temp = vertices_[indices_[3 * f + 1]];
    const Vertex& v2_temp = vertices_[indices_[3 * f + 2]];
    const AxisAlignedBoundingBox& box0_temp = AxisAlignedBoundingBox(v0_temp.position);
    const AxisAlignedBoundingBox& box1_temp = AxisAlignedBoundingBox(v1_temp.position);
    const AxisAlignedBoundingBox& box2_temp = AxisAlignedBoundingBox(v2_temp.position);
    result_box |= (box0_temp | box1_temp | box2_temp);
  }
  return result_box;
}

void AcceleratedMesh::SplitFacesAlongAxis_(const std::string& axis, const std::vector<int>& face_indices, std::vector<int>& result_face_indices_left, std::vector<int>& result_face_indices_right)
{
  auto face_indices_sort = std::vector<int>(face_indices); // A copy used for sort
  if (axis == "x") {
    // Get compare function
    auto compare = [this](int f1, int f2) -> bool {
      const glm::vec3& center1 = GetCenter(f1);
      const glm::vec3& center2 = GetCenter(f2);
      return (center1.x < center2.x);
      };
    std::sort(face_indices_sort.begin(), face_indices_sort.end(), compare);
  }
  else if (axis == "y") {
    auto compare = [this](int f1, int f2) -> bool {
      const glm::vec3& center1 = GetCenter(f1);
      const glm::vec3& center2 = GetCenter(f2);
      return (center1.y < center2.y);
      };
    std::sort(face_indices_sort.begin(), face_indices_sort.end(), compare);
  }
  else if (axis == "z") {
    auto compare = [this](int f1, int f2) -> bool {
      const glm::vec3& center1 = GetCenter(f1);
      const glm::vec3& center2 = GetCenter(f2);
      return (center1.z < center2.z);
      };
    std::sort(face_indices_sort.begin(), face_indices_sort.end(), compare);
  }
  else {
    LAND_ERROR("Unknown axis {}", axis);
  }
  int median_idx = face_indices_sort.size() / 2;
  result_face_indices_left.assign(face_indices_sort.begin(), face_indices_sort.begin() + median_idx);
  result_face_indices_right.assign(face_indices_sort.begin() + median_idx, face_indices_sort.end());
}

glm::vec3 AcceleratedMesh::GetCenter(int face_index)
{
  if (face_index < 0 || face_index >= GetNumFaces()) {
    LAND_ERROR("Face index ({}) out of range [0, {}]", face_index, GetNumFaces());
  }
  const Vertex& v0 = vertices_[indices_[3 * face_index]];
  const Vertex& v1 = vertices_[indices_[3 * face_index + 1]];
  const Vertex& v2 = vertices_[indices_[3 * face_index + 2]];
  return (v0.position + v1.position + v2.position) / 3.0f;
}

void AcceleratedMesh::BuildBvhRecursive_(BvhNode* node, const std::vector<int> & face_indices)
{
  AxisAlignedBoundingBox box = FindBox_(face_indices);
  if (face_indices.size() <= LEAF_FACES_) { // leaf node
    node->content = std::make_unique<BvhNodeData>(box, face_indices, true);
  }
  else { // internal node
    node->content = std::make_unique<BvhNodeData>(box); // Only stores bounding box
    std::string split_axis = box.FindLongestAxis();
    std::vector<int> face_indices_left, face_indices_right;
    SplitFacesAlongAxis_(split_axis, face_indices, face_indices_left, face_indices_right);
    node->left_child = std::make_unique<BvhNode>();
    node->right_child = std::make_unique<BvhNode>();
    BuildBvhRecursive_(node->left_child.get(), face_indices_left);
    BuildBvhRecursive_(node->right_child.get(), face_indices_right);
  }
}

}  // namespace sparks
