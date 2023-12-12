#pragma once
#include "glm/glm.hpp"
#include <random>

namespace sparks {
// Parent class of Common geometries. Used for Light source
class Geometry {
 public:
  // Return the area of the light source
  [[nodiscard]] virtual float GetArea() const = 0; // pure virtual

  // Sample a point on the light source
  [[nodiscard]] virtual glm::vec3 Sample(std::mt19937& rng) const = 0; // pure virtual
};
} // namespace sparks