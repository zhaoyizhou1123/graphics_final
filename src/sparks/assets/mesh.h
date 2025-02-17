#pragma once
#include "sparks/assets/model.h"
#include "sparks/assets/util.h"
#include "sparks/assets/vertex.h"
#include "vector"

namespace sparks {

/* @brief A subclass of Model.

*/
class Mesh : public Model {
 public:
  Mesh() = default;
  Mesh(const Mesh &mesh);
  Mesh(const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices);
  Mesh(const std::vector<Vertex> &vertices,
       const std::vector<uint32_t> &indices,
       const std::vector<glm::vec3>& face_tangents);
  explicit Mesh(const tinyxml2::XMLElement *element);
  ~Mesh() override = default;
  [[nodiscard]] float TraceRay(const glm::vec3 &origin,
                               const glm::vec3 &direction,
                               float t_min,
                               HitRecord *hit_record) const override;
  [[nodiscard]] float TraceRayImprove(const glm::vec3& origin,
    const glm::vec3& direction,
    float t_min,
    float cur_t_min,
    HitRecord* hit_record) const override;
  const char *GetDefaultEntityName() override;
  [[nodiscard]] AxisAlignedBoundingBox GetAABB(
      const glm::mat4 &transform) const override;
  [[nodiscard]] std::vector<Vertex> GetVertices() const override;
  [[nodiscard]] std::vector<uint32_t> GetIndices() const override;
  static Mesh Cube(const glm::vec3 &center, const glm::vec3 &size);
  static Mesh Sphere(const glm::vec3 &center = glm::vec3{0.0f},
                     float radius = 1.0f,
                     float u_freq = 1.0f,
                     float v_freq = 1.0f);
  static bool LoadObjFile(const std::string &obj_file_path, Mesh &mesh);
  void WriteObjFile(const std::string &file_path) const;
  void MergeVertices();
  void Check();
  // Compute the tangent according to (u,v) texture
  void ComputeFaceTangents();

 protected:
  std::vector<Vertex> vertices_;
  // Each entry is an index of vertex. Length is 3f. Faces are (i0,i1,i2); (i3,i4,i5); .... .
  std::vector<uint32_t> indices_;
  std::vector<glm::vec3> face_tangents_; // length is face, each entry is the tangent of the face
};
}  // namespace sparks
