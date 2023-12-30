// https://github.com/mmp/pbrt-v3/
#pragma once 
#include "sparks/util/util.h"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include <random>

namespace sparks {
/* TrowbridgeReitzDistribution
* Assumes all vectors in TBN space (normal = (0,0,1)), need to convert to world space)
*/ 
class MicrofacetDistribution {
public:
  MicrofacetDistribution(float alpha_x, float alpha_y, const glm::vec3& normal)
    : alpha_x_(alpha_x), alpha_y_(alpha_y), normal_(normal) {
    if (glm::abs(glm::length(normal_) - 1.0f) > 1e-5f) {
      LAND_ERROR("Unnormalized normal! {}", glm::to_string(normal_));
    }
  }
  float D(const glm::vec3& wh) const;
  float Lambda(const glm::vec3& w) const;
  float G1(const glm::vec3& w) const {
    //    if (Dot(w, wh) * CosTheta(w) < 0.) return 0.;
    return 1 / (1 + Lambda(w));
  }
  float G(const glm::vec3& wo, const glm::vec3& wi) const {
    return 1 / (1 + Lambda(wo) + Lambda(wi));
  }
  glm::vec3 SampleWh(const glm::vec3& wo, std::mt19937& rng) const;
  float Pdf(const glm::vec3& wo, const glm::vec3& wh) const;

  float RoughnessToAlpha(float roughness) const {
    roughness = glm::max(roughness, (float)1e-3);
    float x = glm::log(roughness);
    return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
      0.000640711f * x * x * x * x;
  }

private:
  // Ensure w is transformed to tangent space
  inline float CosTheta(const glm::vec3& w) const {
    return w.z;
  }
  inline float Cos2Theta(const glm::vec3& w) const { return w.z * w.z; }
  inline float AbsCosTheta(const glm::vec3& w) const { return glm::abs(w.z); }
  inline float Sin2Theta(const glm::vec3& w) const {
    return glm::max((float)0, (float)1 - Cos2Theta(w));
  }

  inline float SinTheta(const glm::vec3& w) const { return glm::sqrt(Sin2Theta(w)); }

  inline float TanTheta(const glm::vec3& w) const { return SinTheta(w) / CosTheta(w); }

  inline float Tan2Theta(const glm::vec3& w) const {
    return Sin2Theta(w) / Cos2Theta(w);
  }

  inline float CosPhi(const glm::vec3& w) const {
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1 : clamp(w.x / sinTheta, -1, 1);
  }

  inline float SinPhi(const glm::vec3& w) const {
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 0 : clamp(w.y / sinTheta, -1, 1);
  }

  inline float Cos2Phi(const glm::vec3& w) const { return CosPhi(w) * CosPhi(w); }

  inline float Sin2Phi(const glm::vec3& w) const { return SinPhi(w) * SinPhi(w); }

  // Tangent space
  void TrowbridgeReitzSample11_(float cosTheta, float u1, float u2,
    float* slope_x, float* slope_y) const;

  glm::mat3 GetTangent2World_() const;

  glm::mat3 GetWorld2Tangent_() const;

  // MicrofacetDistribution private data
  const float alpha_x_, alpha_y_;
  glm::vec3 normal_;
  //glm::mat3 tangent2world_; // convert from tangent space to world space
  //glm::mat3 world2tangent_; // convert from world space to tangent space
};
}
