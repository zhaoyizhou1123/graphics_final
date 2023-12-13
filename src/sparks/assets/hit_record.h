#pragma once
#include "glm/glm.hpp"

namespace sparks {
struct HitRecord {
  int hit_entity_id{-1};
  glm::vec3 position{};
  glm::vec3 normal{}; // A weighted average of the normal stored in the three vertices of triangle
  glm::vec3 geometry_normal{}; // The normal of the triangle in the mesh
  glm::vec3 tangent{};
  glm::vec2 tex_coord{};
  bool front_face{}; // True if hit positive direction of geometry normal (front face of triangle)
};
}  // namespace sparks
