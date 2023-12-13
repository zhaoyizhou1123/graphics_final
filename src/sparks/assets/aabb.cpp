#include "sparks/assets/aabb.h"

#include "algorithm"
#include "grassland/grassland.h"
#include <iostream>

namespace sparks {

AxisAlignedBoundingBox::AxisAlignedBoundingBox(float x_low,
                                               float x_high,
                                               float y_low,
                                               float y_high,
                                               float z_low,
                                               float z_high) {
  this->x_low = x_low;
  this->x_high = x_high;
  this->y_low = y_low;
  this->y_high = y_high;
  this->z_low = z_low;
  this->z_high = z_high;
}

AxisAlignedBoundingBox::AxisAlignedBoundingBox(const glm::vec3 &position) {
  x_low = position.x;
  x_high = position.x;
  y_low = position.y;
  y_high = position.y;
  z_low = position.z;
  z_high = position.z;
}

bool AxisAlignedBoundingBox::IsIntersect(const glm::vec3 &origin,
                                         const glm::vec3 &direction,
                                         float t_min,
                                         float t_max,
                                         float* range_min, 
                                         float* range_max) const {
  bool has_intersect = false;
  // Origin inside box
  if (x_low <= origin.x && origin.x <= x_high && y_low <= origin.y &&
      origin.y <= y_high && z_low <= origin.z && origin.z <= z_high) {
    has_intersect = true;
  }
  if (t_min < 0.0f) {
    LAND_ERROR("t_min should be set non-negative, but got {}", t_min);
  }
  float intersection_range_low = t_max * (1.0f + t_min);
  float intersection_range_high = 0.0f;
  float t;
  glm::vec3 intersection;
#define TestIntersection(x, y, z)                                     \
  if (std::abs(direction.x) > 1e-5) {                                 \
    float inv_d = 1.0f / direction.x;                                 \
    t = (x##_low - origin.x) * inv_d;                                 \
    intersection = origin + direction * t;                            \
    if (y##_low <= intersection.y && intersection.y <= y##_high &&    \
        z##_low <= intersection.z && intersection.z <= z##_high) {    \
      intersection_range_low = std::min(intersection_range_low, t);   \
      intersection_range_high = std::max(intersection_range_high, t); \
    }                                                                 \
    t = (x##_high - origin.x) * inv_d;                                \
    intersection = origin + direction * t;                            \
    if (y##_low <= intersection.y && intersection.y <= y##_high &&    \
        z##_low <= intersection.z && intersection.z <= z##_high) {    \
      intersection_range_low = std::min(intersection_range_low, t);   \
      intersection_range_high = std::max(intersection_range_high, t); \
    }                                                                 \
  }
  TestIntersection(x, y, z);
  TestIntersection(z, x, y);
  TestIntersection(y, z, x);
  // If intersect, always two intersections, then we get a range (min might <0)
  *range_min = std::max(intersection_range_low, t_min); // Assume intersection exists, in case min < t_min
  *range_max = intersection_range_high;
  if (intersection_range_high >= t_min && intersection_range_low <= t_max) { // Case for exterior origin
    has_intersect = true;
  }
  return has_intersect;
}

AxisAlignedBoundingBox AxisAlignedBoundingBox::operator&(
    const AxisAlignedBoundingBox &aabb) const {
  return {std::max(x_low, aabb.x_low), std::min(x_high, aabb.x_high),
          std::max(y_low, aabb.y_low), std::min(y_high, aabb.y_high),
          std::max(z_low, aabb.z_low), std::min(z_high, aabb.z_high)};
}

AxisAlignedBoundingBox AxisAlignedBoundingBox::operator|(
    const AxisAlignedBoundingBox &aabb) const {
  return {std::min(x_low, aabb.x_low), std::max(x_high, aabb.x_high),
          std::min(y_low, aabb.y_low), std::max(y_high, aabb.y_high),
          std::min(z_low, aabb.z_low), std::max(z_high, aabb.z_high)};
}

AxisAlignedBoundingBox &AxisAlignedBoundingBox::operator&=(
    const AxisAlignedBoundingBox &aabb) {
  x_low = std::max(x_low, aabb.x_low);
  x_high = std::min(x_high, aabb.x_high);
  y_low = std::max(y_low, aabb.y_low);
  y_high = std::min(y_high, aabb.y_high);
  z_low = std::max(z_low, aabb.z_low);
  z_high = std::min(z_high, aabb.z_high);
  return *this;
}

AxisAlignedBoundingBox &AxisAlignedBoundingBox::operator|=(
    const AxisAlignedBoundingBox &aabb) {
  x_low = std::min(x_low, aabb.x_low);
  x_high = std::max(x_high, aabb.x_high);
  y_low = std::min(y_low, aabb.y_low);
  y_high = std::max(y_high, aabb.y_high);
  z_low = std::min(z_low, aabb.z_low);
  z_high = std::max(z_high, aabb.z_high);
  return *this;
}

bool AxisAlignedBoundingBox::operator==(const AxisAlignedBoundingBox& aabb) const {
  return (x_low == aabb.x_low) && (x_high == aabb.x_high)
    && (y_low == aabb.y_low) && (y_high == aabb.y_high)
    && (z_low == aabb.z_low) && (z_high == aabb.z_high);
}

std::string AxisAlignedBoundingBox::FindLongestAxis()
{
  float temp_longest_range = x_high - x_low;
  std::string temp_axis = "x";
  
  float y_range = y_high - y_low;
  if (temp_longest_range < y_range) {
    temp_longest_range = y_range;
    temp_axis = "y";
  }
  float z_range = z_high - z_low;
  if (temp_longest_range < z_range) {
    temp_longest_range = z_range;
    temp_axis = "z";
  }
  return temp_axis;
}

void AxisAlignedBoundingBox::ShowBox() const {
  std::cout << "Box: (" << x_low << ", " << x_high << ") x ";
  std::cout << "(" << y_low << ", " << y_high << ") x ";
  std::cout << "(" << z_low << ", " << z_high << ")\n";
}

}  // namespace sparks
