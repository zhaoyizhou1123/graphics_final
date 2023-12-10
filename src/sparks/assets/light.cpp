#include "light.h"
#include <random>

float sparks::Lights::GetTotalArea() const
{
	return total_area_;
}

glm::vec3 sparks::Lights::Sample(int seed_x, int seed_y, int seed_z) const
{	
	// Sample a light source according to area
	std::mt19937 rng_idx(seed_x ^ seed_y ^ seed_z);
	int idx = std::discrete_distribution<int>(areas_.begin(), areas_.end())(rng_idx);

	auto light = lights_[idx];
	return light.geometry->Sample(seed_x, seed_y, seed_z);
}

//void sparks::Lights::AddLight(const Light& light)
//{
//	lights_.emplace_back(std::move(light));
//	total_area_ += light.geometry->GetArea();
//}
//
//void sparks::Lights::AddLight(Light&& light)
//{
//	lights_.push_back(std::move(light));
//	total_area_ += light.geometry->GetArea();
//}

void sparks::Lights::AddLight(Geometry* geometry, const glm::vec3 & emission, float emission_strength)
{
	lights_.emplace_back(geometry, emission, emission_strength);
	areas_.emplace_back(geometry->GetArea());
	total_area_ += geometry->GetArea();
}

sparks::Light sparks::Lights::GetLight(int idx) const {
	return lights_[idx];
}
