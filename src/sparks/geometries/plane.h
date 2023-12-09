#pragma once
#include "sparks/geometries/geometry.h"
#include <vector>

namespace sparks {
class Plane : public Geometry {
public:
	// @param xyz_order: "xzy" means coord0=x, 1=z, 2=y
	Plane(
		float coord0_min,
		float coord0_max,
		float coord1_min,
		float coord1_max,
		float coord2,
		const char* xyz_order);
	[[nodiscard]] float GetArea() const;
	[[nodiscard]] glm::vec3 Sample(int seed0, int seed1) const;
private:
	// Define a rectangle
	float coord0_min_;
	float coord0_max_;
	float coord1_min_;
	float coord1_max_;

	// The rest dimension
	float coord2_;

	// Map x,y,z dim to coordinate index. E.g., (2,0,1) means x dim is 2, y dim is 1, z dim is 0
	std::vector<int> xyz_2_coord_index_; 

	float area_;

};
} // namespace sparks