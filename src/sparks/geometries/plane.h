#pragma once
#include "sparks/geometries/geometry.h"
#include <vector>
#include "tinyxml2.h"

namespace sparks {
class Plane : public Geometry {
public:
	Plane(): x_min_{0.0f}, x_max_{0.0f}, y_min_{0.0f}, y_max_{0.0f}, z_min_{0.0f}, z_max_{0.0f}, area_{0.0f} {}
	Plane(const tinyxml2::XMLElement* element);
	[[nodiscard]] float GetArea() const;
	
	// Only two seeds will be used
	[[nodiscard]] glm::vec3 Sample(std::mt19937& rng) const;
private:
	// Define a rectangle (On one dim, min == max)
	float x_min_, x_max_, y_min_, y_max_, z_min_, z_max_;

	float area_;

};
} // namespace sparks