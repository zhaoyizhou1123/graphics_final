#pragma once
#include "sparks/geometries/geometry.h"
#include <vector>
#include "tinyxml2.h"

namespace sparks {
class Plane : public Geometry {
public:
	Plane(const tinyxml2::XMLElement* element);
	[[nodiscard]] float GetArea() const;
	
	// Only two seeds will be used
	[[nodiscard]] glm::vec3 Sample(int seed_x, int seed_y, int seed_z) const;
private:
	// Define a rectangle (On one dim, min == max)
	float x_min_, x_max_, y_min_, y_max_, z_min_, z_max_;

	float area_;

};
} // namespace sparks