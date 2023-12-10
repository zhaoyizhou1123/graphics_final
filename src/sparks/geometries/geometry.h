#pragma once
#include "glm/glm.hpp"

namespace sparks {
// Parent class of Common geometries. Used for Light source
class Geometry {
 public:
  // Return the area of the light source
  [[nodiscard]] virtual float GetArea() const = 0; // pure virtual

  // Sample a point on the light source
  [[nodiscard]] virtual glm::vec3 Sample(int seed_x, int seed_y, int seed_z) const = 0; // pure virtual
};
} // namespace sparks