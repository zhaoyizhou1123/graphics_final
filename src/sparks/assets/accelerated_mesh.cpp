#include "sparks/assets/accelerated_mesh.h"

#include "algorithm"
#include <numeric>
#include <glm/gtx/string_cast.hpp>

namespace sparks {
  AcceleratedMesh::AcceleratedMesh(const Mesh& mesh, bool use_accelerate) : 
    Mesh(mesh), use_accelerate_{ use_accelerate } {
  BuildAccelerationStructure();
}

AcceleratedMesh::AcceleratedMesh(const std::vector<Vertex> &vertices,
                                 const std::vector<uint32_t> &indices,
                                 bool use_accelerate)
  : Mesh(vertices, indices), use_accelerate_{ use_accelerate } {
  BuildAccelerationStructure();
}

float AcceleratedMesh::TraceRay(const glm::vec3 &origin,
                                const glm::vec3 &direction,
                                float t_min,
                                HitRecord *hit_record) const {
  if (!use_accelerate_) { // Do not use acceleration structure
    return Mesh::TraceRay(origin, direction, t_min, hit_record);
  }
  // Use acceleration structure
  return TraceRayRecursive_(
    bvh_->GetRoot(),
    -1.0f,
    origin,
    direction,
    t_min,
    hit_record);
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
  int f_idx_temp = face_indices[0];
  const Vertex& v0 = vertices_[indices_[3*f_idx_temp]];
  const Vertex& v1 = vertices_[indices_[3 * f_idx_temp + 1]];
  const Vertex& v2 = vertices_[indices_[3 * f_idx_temp + 2]];
  const AxisAlignedBoundingBox& box0 = AxisAlignedBoundingBox(v0.position);
  const AxisAlignedBoundingBox& box1 = AxisAlignedBoundingBox(v1.position);
  const AxisAlignedBoundingBox& box2 = AxisAlignedBoundingBox(v2.position);
  AxisAlignedBoundingBox result_box = box0 | box1 | box2;
  for (int i = 1; i < face_indices.size(); i++) {
    int f_idx = face_indices[i];
    const Vertex& v0_temp = vertices_[indices_[3 * f_idx]];
    const Vertex& v1_temp = vertices_[indices_[3 * f_idx + 1]];
    const Vertex& v2_temp = vertices_[indices_[3 * f_idx + 2]];
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
      const glm::vec3& center1 = GetCenter_(f1);
      const glm::vec3& center2 = GetCenter_(f2);
      return (center1.x < center2.x);
      };
    std::sort(face_indices_sort.begin(), face_indices_sort.end(), compare);
  }
  else if (axis == "y") {
    auto compare = [this](int f1, int f2) -> bool {
      const glm::vec3& center1 = GetCenter_(f1);
      const glm::vec3& center2 = GetCenter_(f2);
      return (center1.y < center2.y);
      };
    std::sort(face_indices_sort.begin(), face_indices_sort.end(), compare);
  }
  else if (axis == "z") {
    auto compare = [this](int f1, int f2) -> bool {
      const glm::vec3& center1 = GetCenter_(f1);
      const glm::vec3& center2 = GetCenter_(f2);
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

glm::vec3 AcceleratedMesh::GetCenter_(int face_index)
{
  if (face_index < 0 || face_index >= GetNumFaces()) {
    LAND_ERROR("Face index ({}) out of range [0, {}]", face_index, GetNumFaces());
  }
  const Vertex& v0 = vertices_[indices_[3 * face_index]];
  const Vertex& v1 = vertices_[indices_[3 * face_index + 1]];
  const Vertex& v2 = vertices_[indices_[3 * face_index + 2]];
  return (v0.position + v1.position + v2.position) / 3.0f;
}

float AcceleratedMesh::TraceRayRecursive_(BvhNode* cur_node, float cur_t_min, const glm::vec3& origin, const glm::vec3& direction, float t_min, HitRecord* hit_record) const
{
//  LAND_INFO("Test Intersection with box ({},{}) x ({},{}) x ({},{})",
//    cur_node->content->box.x_low, cur_node->content->box.x_high,
//    cur_node->content->box.y_low, cur_node->content->box.y_high,
//    cur_node->content->box.z_low, cur_node->content->box.z_high);
  float range_min, range_max;
  bool has_intersect = cur_node->content->box.IsIntersect(origin, direction, t_min, 1e5, &range_min, &range_max);
  // No intersection, return the original result
  if (!has_intersect) {
    //LAND_INFO("No intersection, return.");
    return cur_t_min;
  }
  // Has intersection, previous result exists and cannot get nearer result in this box, return the original result
  if (cur_t_min >= t_min && range_min >= cur_t_min) {
    return cur_t_min;
  }
  // Find intersection in this box
  if (cur_node->content->is_leaf) { // Is a leaf
    //LAND_INFO("Test leaf node");
    HitRecord local_hit_record;
    float t_temp = TraceRayLeaf_(
      cur_node->content->face_indices,
      origin,
      direction,
      t_min,
      hit_record? &local_hit_record : nullptr
    );
    // This leaf has intersection, and previous intersection does not exist, or new result is smaller
    if ((t_temp >= t_min) && ((cur_t_min < t_min) || (t_temp < cur_t_min))) {
      if (hit_record != nullptr) {
        *hit_record = local_hit_record;
      }
      return t_temp;
    }
    else {
      return cur_t_min;
    }
  }
  else { // Is an internal node
    // A better idea is to test the potentially nearer child node
    float left_t_min = TraceRayRecursive_(
      cur_node->left_child.get(),
      cur_t_min,
      origin,
      direction,
      t_min,
      hit_record);
    float right_t_min = TraceRayRecursive_(
      cur_node->right_child.get(),
      left_t_min,
      origin,
      direction,
      t_min,
      hit_record);
    return right_t_min;
  }
}

float AcceleratedMesh::TraceRayLeaf_(const std::vector<int>& face_indices, const glm::vec3& origin, const glm::vec3& direction, float t_min, HitRecord* hit_record) const
{
  float result = -1.0f;
  for (int idx = 0; idx < face_indices.size(); idx++) { // iterate through all triangles
    int face_idx = face_indices[idx];
    const auto& v0 = vertices_[indices_[3 * face_idx]];
    const auto& v1 = vertices_[indices_[3 * face_idx + 1]];
    const auto& v2 = vertices_[indices_[3 * face_idx + 2]];
    //LAND_INFO("Test intersection with triangle {} - {} - {}",
    //  glm::to_string(v0.position),
    //  glm::to_string(v1.position),
    //  glm::to_string(v2.position));

    glm::mat3 A = glm::mat3(v1.position - v0.position,
      v2.position - v0.position, -direction);
    if (std::abs(glm::determinant(A)) < 1e-9f) {
      continue;
    }
    A = glm::inverse(A);
    auto uvt = A * (origin - v0.position);
    auto& t = uvt.z;
    if (t < t_min || (result > 0.0f && t > result)) {
      continue;
    }
    auto& u = uvt.x;
    auto& v = uvt.y;
    auto w = 1.0f - u - v;
    auto position = origin + t * direction;
    if (u >= 0.0f && v >= 0.0f && u + v <= 1.0f) {
      result = t;
      if (hit_record) {
        auto geometry_normal = glm::normalize(
          glm::cross(v1.position - v0.position, v2.position - v0.position)); // Respect the order
        if (glm::dot(geometry_normal, v0.normal) < 0) { // opposite direction 
          LAND_ERROR("Opposite direction at v0 {}, v1 {}, v2 {}: geometry {}, normal {}",
            glm::to_string(v0.position),
            glm::to_string(v1.position),
            glm::to_string(v2.position),
            glm::to_string(geometry_normal),
            glm::to_string(v0.normal));
        }
        if (glm::dot(geometry_normal, direction) < 0.0f) {
          hit_record->position = position;
          hit_record->geometry_normal = geometry_normal;
          hit_record->normal = v0.normal * w + v1.normal * u + v2.normal * v;
          hit_record->tangent =
            v0.tangent * w + v1.tangent * u + v2.tangent * v;
          hit_record->tex_coord =
            v0.tex_coord * w + v1.tex_coord * u + v2.tex_coord * v;
          hit_record->front_face = true;
        }
        else {
          hit_record->position = position;
          hit_record->geometry_normal = -geometry_normal;
          hit_record->normal = -(v0.normal * w + v1.normal * u + v2.normal * v);
          hit_record->tangent =
            -(v0.tangent * w + v1.tangent * u + v2.tangent * v);
          hit_record->tex_coord =
            v0.tex_coord * w + v1.tex_coord * u + v2.tex_coord * v;
          hit_record->front_face = false;
        }
      }
    }
  }
  //if (result > -1.0f) {
  //  //LAND_INFO("Get intersection with distance {}", result);
  //}
  return result;
}

void AcceleratedMesh::BuildBvhRecursive_(BvhNode* node, const std::vector<int> & face_indices)
{
  AxisAlignedBoundingBox box = FindBox_(face_indices);
  //LAND_INFO("{} Faces. Box ({},{}) x ({},{}) x ({},{})", face_indices.size(),
  //  box.x_low, box.x_high,
  //  box.y_low, box.y_high,
  //  box.z_low, box.z_high);
  if (face_indices.size() <= LEAF_FACES_) { // leaf node
    node->content = std::make_unique<BvhNodeData>(box, face_indices, true);
    //LAND_INFO("Leaf");
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

    //const AxisAlignedBoundingBox& box_left = node->left_child->content->box;
    //const AxisAlignedBoundingBox& box_right = node->right_child->content->box;
    //const AxisAlignedBoundingBox& box_self = node->content->box;
    //if (!(box_self == (box_left | box_right))) {
    //  std::cout << "Left box: ";
    //  box_left.ShowBox();
    //  std::cout << "Right box: ";
    //  box_right.ShowBox();
    //  std::cout << "Parent box: ";
    //  box_self.ShowBox();
    //  std::cout << "Left faces: ";
    //  show_vector(face_indices_left);
    //  std::cout << "Right faces: ";
    //  show_vector(face_indices_right);
    //  std::cout << "Parent faces: ";
    //  show_vector(face_indices);
    //  LAND_ERROR("Detected incorrect box assignment!");
    //}
  }
}

}  // namespace sparks
