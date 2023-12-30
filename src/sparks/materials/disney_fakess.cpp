#include "disney_fakess.h"

float sparks::DisneyFakess::GetPdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const
{
	float cos_theta = glm::dot(normal, -ray_in);
	if (cos_theta < 0) { // opposite direction
		return 0.0f;
	}
	else {
		return cos_theta * INV_PI;
	}
}

// based on https://github.com/mmp/pbrt-v3/
glm::vec3 sparks::DisneyFakess::GetBsdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const
{
	glm::vec3 ray_in_reverse = -ray_in;
	glm::vec3 ray_half = ray_out + ray_in_reverse;
	if (glm::all(glm::equal(ray_half, glm::vec3{ 0.0f }))) {
		return glm::vec3{ 0.0f };
	}
	ray_half = glm::normalize(ray_half);
	float cos_theta_d = glm::dot(ray_out, ray_half);

	float fss90 = cos_theta_d * cos_theta_d * roughness_;
	float cos_theta_out = glm::abs(glm::dot(normal, ray_out));
	float cos_theta_in = glm::abs(glm::dot(normal, ray_in_reverse));
	float f_out = pow5(cos_theta_out);
	float f_in = pow5(cos_theta_in);
	float fss = linear_interpolate(f_out, 1.0f, fss90) * linear_interpolate(f_in, 1.0f, fss90);

	float ss = 1.25f * (fss * (1 / (cos_theta_out + cos_theta_in) - 0.5f) + 0.5f);
	return base_color_ * INV_PI * ss;
}

glm::vec3 sparks::DisneyFakess::SampleRayIn(const glm::vec3& normal, const glm::vec3& ray_out, glm::vec3* ray_in, float* pdf, std::mt19937& rng, bool is_front_face) const
{
	//if (glm::dot(normal, ray_out) < 0) {
	//	LAND_WARN("Opposite direction of out ray and normal! {}", glm::dot(normal, ray_out));
	//}
	glm::vec3 ray_in_reverse = hemisphere_sample_cosine_weighted(normal, rng, pdf);
	*ray_in = -ray_in_reverse;
	return GetBsdf(normal, ray_out, *ray_in, is_front_face);
}
