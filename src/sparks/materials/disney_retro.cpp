#include "disney_retro.h"

float sparks::DisneyRetro::GetPdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const
{
	float cos_theta = glm::dot(normal, -ray_in);
	if (cos_theta < 0) { // opposite direction
		return 0.0f;
	}
	else {
		return cos_theta * INV_PI;
	}
}

glm::vec3 sparks::DisneyRetro::GetBsdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const
{
	glm::vec3 ray_in_reverse = -ray_in;
	glm::vec3 ray_half = ray_out + ray_in_reverse;
	if (glm::all(glm::equal(ray_half, glm::vec3{ 0.0f }))) {
		return glm::vec3{ 0.0f };
	}
	ray_half = glm::normalize(ray_half);
	float cos_theta_d = glm::dot(ray_out, ray_half);
	float cos_theta_out = glm::abs(glm::dot(normal, ray_out));
	float cos_theta_in = glm::abs(glm::dot(normal, ray_in_reverse));
	float f_out = pow5(cos_theta_out);
	float f_in = pow5(cos_theta_in);
	float rr = 2 * roughness_ * cos_theta_d * cos_theta_d; // Rr

	return base_color_ * INV_PI * rr * (f_out + f_in + f_out * f_in * (rr - 1));
}

glm::vec3 sparks::DisneyRetro::SampleRayIn(const glm::vec3& normal, const glm::vec3& ray_out, glm::vec3* ray_in, float* pdf, std::mt19937& rng, bool is_front_face) const
{
	glm::vec3 ray_in_reverse = hemisphere_sample_cosine_weighted(normal, rng, pdf);
	*ray_in = -ray_in_reverse;
	return GetBsdf(normal, ray_out, *ray_in, is_front_face);
}
