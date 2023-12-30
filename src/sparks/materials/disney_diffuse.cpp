#include "disney_diffuse.h"

namespace sparks {
float DisneyDiffuse::GetPdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const
{
	float cos_theta = glm::dot(normal, -ray_in);
	if (cos_theta < 0) { // opposite direction
		return 0.0f;
	}
	else {
		return cos_theta * INV_PI;
	}
}

glm::vec3 DisneyDiffuse::GetBsdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const
{
	if (glm::abs(glm::length(normal) - 1.0f) > 1e-3f) {
		LAND_ERROR("Unnormalized normal {}", glm::to_string(normal));
	}
	if (glm::abs(glm::length(ray_out) - 1.0f) > 1e-3f) {
		LAND_ERROR("Unnormalized ray out {}", glm::to_string(ray_out));
	}
	if (glm::abs(glm::length(ray_in) - 1.0f) > 1e-3f) {
		LAND_ERROR("Unnormalized ray in {}", glm::to_string(ray_in));
	}
	float cos_theta_out = glm::abs(glm::dot(normal, ray_out));
	float cos_theta_in = glm::abs(glm::dot(normal, ray_in));
	float f_out = pow5(cos_theta_out);
	float f_in = pow5(cos_theta_in);
	return base_color_ * INV_PI * (1 - f_out / 2) * (1 - f_in / 2);
}

glm::vec3 DisneyDiffuse::SampleRayIn(const glm::vec3& normal, const glm::vec3& ray_out, glm::vec3* ray_in, float* pdf, std::mt19937& rng, bool is_front_face) const
{
	//if (glm::dot(normal, ray_out) < 0) {
	//	LAND_WARN("Opposite direction of out ray and normal! {}", glm::dot(normal, ray_out));
	//}
	glm::vec3 ray_in_reverse = hemisphere_sample_cosine_weighted(normal, rng, pdf);
	*ray_in = -ray_in_reverse;
	return GetBsdf(normal, ray_out, *ray_in, is_front_face);
}
} // namespace sparks