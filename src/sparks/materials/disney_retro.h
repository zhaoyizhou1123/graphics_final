#pragma once
#include "bxdf.h"

namespace sparks {
  class DisneyRetro : public Bxdf {
  public:
    DisneyRetro(const glm::vec3& base_color, float roughness) :
      Bxdf(BxdfType(BSDF_DIFFUSE | BSDF_SPECULAR)), base_color_(base_color), roughness_(roughness) {}
    float GetPdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const;
    glm::vec3 GetBsdf(
      const glm::vec3& normal,
      const glm::vec3& ray_out,
      const glm::vec3& ray_in, bool is_front_face) const;
    glm::vec3 SampleRayIn(
      const glm::vec3& normal,
      const glm::vec3& ray_out,
      glm::vec3* ray_in,
      float* pdf,
      std::mt19937& rng, bool is_front_face) const;
  private:
    glm::vec3 base_color_;
    float roughness_;
  };
} // namespace sparkss