#pragma once
#include "glm/glm.hpp"
#include <random>

namespace sparks {
/* @brief Sample from a unit hemisphere.
* @param normal: The normal of the point, used to determine the hemisphere
* @return sampled ray. Currently adopt uniform sampling
*/
glm::vec3 hemisphere_sample(const glm::vec3 &normal, std::mt19937& rng);

/* @brief Unit hemisphere sampling on standard coordinates (x,y,z), with z >=0
*/
glm::vec3 hemisphere_sample_std(std::mt19937& rng);

/* @brief convert normal space (coordinate) to world space (standard (x,y,z) coordinate)
*/
glm::vec3 normal2world(const glm::vec3 & normal, const glm::vec3 & p);
} // namespace sparks