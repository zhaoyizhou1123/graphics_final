#include "light.h"
#include <random>

namespace sparks {
float Lights::GetTotalArea() const
{
	return total_area_;
}

void Lights::Sample(int* light_idx, glm::vec3* pos, std::mt19937& rng) const
{
	// Sample a light source according to area
	int idx = std::discrete_distribution<int>(areas_.begin(), areas_.end())(rng);
	*light_idx = idx;

	const Light* light = &lights_[idx];
	if (light->geometry == nullptr) {
		LAND_ERROR("Geometry is nullptr!");
		throw "Nullptr geometry!";
	}
	*pos = light->geometry->Sample(rng);
}

//void Lights::AddLight(const Light& light)
//{
//	lights_.emplace_back(std::move(light));
//	total_area_ += light.geometry->GetArea();
//}
//
//void Lights::AddLight(Light&& light)
//{
//	lights_.push_back(std::move(light));
//	total_area_ += light.geometry->GetArea();
//}

void Lights::AddLight(std::unique_ptr<Geometry>&& geometry, const glm::vec3& emission, float emission_strength)
{
	areas_.emplace_back(geometry->GetArea());
	total_area_ += geometry->GetArea();
	lights_.emplace_back(std::move(geometry), emission, emission_strength);
}

const Light* Lights::GetLight(int idx) const {
	return &(lights_[idx]);
}
} // namespace sparks

