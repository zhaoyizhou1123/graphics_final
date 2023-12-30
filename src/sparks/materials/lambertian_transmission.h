#pragma once
#include "bxdf.h"

namespace sparks {
  class LambertianTransmission : public Bxdf {
  public:
    LambertianTransmission(const glm::vec3& base_color) :
      Bxdf(BxdfType(BSDF_TRANSMISSIVE | BSDF_DIFFUSE)), base_color_(base_color) {}
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
  };
} // namespace sparkss