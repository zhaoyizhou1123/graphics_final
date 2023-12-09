#pragma once
#include "glm/glm.hpp"

namespace sparks {
// Parent class of Common geometries. Used for Light source
class Geometry {
 public:
  // Return the area of the light source
  [[nodiscard]] virtual float GetArea() const;

  // Sample a point on the light source
  [[nodiscard]] virtual glm::vec3 Sample(int seed0, int seed1) const;
};
} // namespace sparks