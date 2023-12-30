// Based on https://github.com/mmp/pbrt-v3/
#pragma once
#include "sparks/util/sample.h"
#include "sparks/util/util.h"
#include "sparks/assets/assets.h"

namespace sparks {
enum BxdfType : int {
  BSDF_DIFFUSE = 1 << 0,
  BSDF_SPECULAR = 1 << 1,
  BSDF_TRANSMISSIVE = 1 << 2,
};

class Bxdf {
public:
  Bxdf(BxdfType type) : bxdf_type{ type } {}
  virtual float GetPdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const = 0;
  virtual glm::vec3 GetBsdf(
    const glm::vec3& normal,
    const glm::vec3& ray_out,
    const glm::vec3& ray_in, bool is_front_face) const = 0;

  // @return BSDF
  virtual glm::vec3 SampleRayIn(
    const glm::vec3& normal,
    const glm::vec3& ray_out,
    glm::vec3* ray_in,
    float* pdf,
    std::mt19937& rng, bool is_front_face) const = 0;

  const BxdfType bxdf_type;
};
} // namespace sparks
