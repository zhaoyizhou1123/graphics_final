#include "lambertian_transmission.h"

namespace sparks {
float LambertianTransmission::GetPdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const
{
	if (glm::dot(normal, -ray_in) * glm::dot(normal, ray_out) > 0) { // same direction
		return 0.0f;
	}
	else {
		return glm::abs(glm::dot(normal, -ray_in)) * INV_PI;
	}
}

glm::vec3 LambertianTransmission::GetBsdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const
{
	return base_color_ * INV_PI;
}

glm::vec3 LambertianTransmission::SampleRayIn(const glm::vec3& normal, const glm::vec3& ray_out, glm::vec3* ray_in, float* pdf, std::mt19937& rng, bool is_front_face) const
{
	glm::vec3 ray_in_reverse = hemisphere_sample_cosine_weighted(normal, rng, pdf);
	if (glm::dot(normal, ray_in_reverse) * glm::dot(normal, ray_out) > 0) { // same direction
		*ray_in = ray_in_reverse;
	}
	else {
		*ray_in = -ray_in_reverse;
	}
	return GetBsdf(normal, ray_out, *ray_in, is_front_face);
}
} // namespace sparks

