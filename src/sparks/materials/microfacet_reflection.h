// https://github.com/mmp/pbrt-v3/
#pragma once
#include "bxdf.h"
#include "sparks/util/distribution.h"

namespace sparks {
float fr_dielectric(float cosThetaI, float etaI, float etaT); 

// DisneyFresnel
class Fresnel{
public:
  Fresnel(const glm::vec3& R0, float metallic, float ior)
    : R0(R0), metallic_(metallic), ior_(ior) {}
  glm::vec3 Evaluate(float cosI) const {
    return linear_interpolate(
      metallic_, 
      glm::vec3(fr_dielectric(cosI, 1, ior_)),
      linear_interpolate(pow5(cosI), R0, glm::vec3{ 1.0f }));
  }

private:
  const glm::vec3 R0;
  const float metallic_, ior_;
};

class MicrofacetReflection : public Bxdf {
public:
  MicrofacetReflection(const glm::vec3& base_color,
    std::unique_ptr<MicrofacetDistribution> distribution, std::unique_ptr<Fresnel> fresnel) :
    Bxdf(BxdfType(BSDF_SPECULAR)), base_color_(base_color), distribution_(std::move(distribution)), fresnel_(std::move(fresnel)) {}
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
  //glm::vec3 World2Tangent_(const glm::vec3& normal, const glm::vec3& w);
  const glm::vec3 base_color_;
  std::unique_ptr<MicrofacetDistribution> distribution_;
  std::unique_ptr<Fresnel> fresnel_;
};

class FresnelDielectric{
public:
  // FresnelDielectric Public Methods
  glm::vec3 Evaluate(float cosThetaI) const {
    return glm::vec3{ fr_dielectric(cosThetaI, ior_in_, ior_trans_) };
  }
  FresnelDielectric(float ior_in, float ior_trans) : ior_in_(ior_in), ior_trans_(ior_trans) {}

private:
  float ior_in_, ior_trans_;
};

// The normal here always points to
class MicrofacetTransmission : public Bxdf {
public:
  MicrofacetTransmission(const glm::vec3& base_color,
    std::unique_ptr<MicrofacetDistribution> distribution, float ior_a, float ior_b) :
    Bxdf(BxdfType(BSDF_SPECULAR)), base_color_(base_color), distribution_(std::move(distribution)), 
    ior_a_(ior_a), ior_b_(ior_b), fresnel_(ior_a, ior_b) {}
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
  //glm::vec3 World2Tangent_(const glm::vec3& normal, const glm::vec3& w);
  const glm::vec3 base_color_;
  std::unique_ptr<MicrofacetDistribution> distribution_;
  const FresnelDielectric fresnel_;
  const float ior_a_, ior_b_; // air, material ior
};
} // namespace sparkss