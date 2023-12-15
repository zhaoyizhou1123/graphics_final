#include "sample.h"
#include "sparks/util/util.h"
#include <random>

namespace sparks {
glm::vec3 hemisphere_sample(const glm::vec3 &normal, std::mt19937& rng)
{
	return normal2world(normal, hemisphere_sample_std(rng));
}
glm::vec3 hemisphere_sample_std(std::mt19937& rng)
{
	// Based on https://ameye.dev/notes/sampling-the-hemisphere/
	float eps0 = std::uniform_real_distribution<float>(0.0f, 1.0f)(rng);
	float eps1 = std::uniform_real_distribution<float>(0.0f, 1.0f)(rng);
	float cos_theta = eps0;
	float sin_theta = glm::sqrt(1 - cos_theta * cos_theta);
	float phi = 2 * sparks::PI * eps1;
	float cos_phi = glm::cos(phi);
	float sin_phi = glm::sin(phi);
	return glm::vec3(cos_phi * sin_theta, sin_phi * sin_theta, cos_theta);
}
glm::vec3 normal2world(const glm::vec3 & normal, const glm::vec3 & p)
{
	if (glm::abs(glm::length(normal) - 1.0f) > 1e-3) {
		throw "Unnormalized normal!";
	}
	if (glm::distance(normal, glm::vec3{ 0,0,1 }) < 1e-9) { // normal is same as positive z, no need to rotate
		return p;
	}
	glm::vec3 l = glm::normalize(glm::vec3{ normal.y, -normal.x, 0.0f });
	glm::vec3 m = glm::normalize(glm::vec3{
		normal.x * normal.z,
		normal.y * normal.z,
		-normal.x * normal.x - normal.y * normal.y });
	glm::mat3 rotate = { m,l,normal };
	return rotate * p;
}

// Based on https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations#ConcentricSampleDisk
glm::vec3 hemisphere_sample_cosine_weighted(const glm::vec3& normal, std::mt19937& rng, float* pdf)
{
	// Sample on standard unit hemisphere (normal = (0,0,1))
	glm::vec2 u = disk_sample(rng);
	float z = std::sqrt(std::max(0.0f, 1 - u.x * u.x - u.y * u.y));

	glm::vec3 p_std = glm::vec3{ u.x, u.y, z };

	// Compute pdf
	*pdf = z * INV_PI; // cos_theta = z
	
	// Convert to true hemisphere defined by normal
	return normal2world(normal, p_std);
}

// Based on https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations#ConcentricSampleDisk
glm::vec2 disk_sample(std::mt19937& rng)
{
	// Uniform sampling in [-1,1]x[-1,1]
	float x = std::uniform_real_distribution<float>(-1.0f, 1.0f)(rng);
	float y = std::uniform_real_distribution<float>(-1.0f, 1.0f)(rng);

	if (std::abs(x) < 1e-5 && std::abs(y) < 1e-5) {
		return glm::vec2{ 0.0f };
	}

	float r, theta;
	if (std::abs(x) > std::abs(y)) {
		r = x;
		theta = PI / 4 * (y / x);
	}
	else {
		r = y;
		theta = PI / 2 - PI / 4 * (x / y);
	}
	return glm::vec2{ r * std::cos(theta), r * std::sin(theta) };
}
} // namespace sparks

