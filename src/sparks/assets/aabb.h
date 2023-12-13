#pragma once
#include "glm/glm.hpp"
#include <string>

namespace sparks {
struct AxisAlignedBoundingBox {
  float x_low{};
  float x_high{};
  float y_low{};
  float y_high{};
  float z_low{};
  float z_high{};
  AxisAlignedBoundingBox(float x_low,
                         float x_high,
                         float y_low,
                         float y_high,
                         float z_low,
                         float z_high);
  AxisAlignedBoundingBox(const glm::vec3 &position = glm::vec3{0.0f});
  /*@brief Test intersection
  * @param t_min, t_max. Predefined min and max distance (like 1e-3, 1e4)
  * @param range_min, range_max. Return the range of intersection if intersection exists;
  * Otherwise they are meaning less
  */
  [[nodiscard]] bool IsIntersect(const glm::vec3 &origin,
                                 const glm::vec3 &direction,
                                 float t_min,
                                 float t_max,
                                 float* range_min,
                                 float* range_max) const;
  // Box merge operations
  AxisAlignedBoundingBox operator&(const AxisAlignedBoundingBox &aabb) const; // intersection
  AxisAlignedBoundingBox operator|(const AxisAlignedBoundingBox &aabb) const; // union
  AxisAlignedBoundingBox &operator&=(const AxisAlignedBoundingBox &aabb);
  AxisAlignedBoundingBox &operator|=(const AxisAlignedBoundingBox &aabb);

  bool operator==(const AxisAlignedBoundingBox& aabb) const; // union

  void ShowBox() const;

  // Return the longest axis of box, 'x' or 'y' or 'z'
  std::string FindLongestAxis();
};
}  // namespace sparks
