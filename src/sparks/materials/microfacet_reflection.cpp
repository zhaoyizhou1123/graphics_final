#include "microfacet_reflection.h"
#include <algorithm>

namespace sparks {
float fr_dielectric(float cosThetaI, float etaI, float etaT) {
  cosThetaI = clamp(cosThetaI, -1.0f, 1.0f);

  // Potentially swap indices of refraction
  bool entering = cosThetaI > 0.0f;
  if (!entering) {
    std::swap(etaI, etaT);
    cosThetaI = glm::abs(cosThetaI);
  }

  // Compute _cosThetaT_ using Snell's law
  float sinThetaI = glm::sqrt(glm::max(0.0f, 1.0f - cosThetaI * cosThetaI));
  float sinThetaT = etaI / etaT * sinThetaI;

  // Handle total internal reflection
  if (sinThetaT >= 1) return 1;
  float cosThetaT = glm::sqrt(glm::max(0.0f, 1.0f - sinThetaT * sinThetaT));
  float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
    ((etaT * cosThetaI) + (etaI * cosThetaT));
  float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
    ((etaI * cosThetaI) + (etaT * cosThetaT));
  return (Rparl * Rparl + Rperp * Rperp) / 2;
}

float MicrofacetReflection::GetPdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const
{
  glm::vec3 ray_in_reverse = -ray_in;
  if (glm::dot(normal, ray_in_reverse) * glm::dot(normal, ray_out) < 0.0f) return 0;
  glm::vec3 ray_half = glm::normalize(ray_out + ray_in_reverse);
  return distribution_->Pdf(ray_out, ray_half) / (4 * glm::dot (ray_out, ray_half));
}

glm::vec3 MicrofacetReflection::GetBsdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const
{
  glm::vec3 ray_in_reverse = -ray_in;
  float cos_theta_out = glm::abs(glm::dot(normal, ray_out));
  float cos_theta_in = glm::abs(glm::dot(normal, ray_in_reverse));
  glm::vec3 ray_half = ray_in_reverse + ray_out;
  // Handle degenerate cases for microfacet reflection
  if (cos_theta_in == 0 || cos_theta_out == 0) return glm::vec3{ 0.0f };
  if (glm::all(glm::equal(ray_half, glm::vec3{0.0f}))) return glm::vec3{0.0f};
  ray_half = glm::normalize(ray_half);
  // For the Fresnel call, make sure that ray_half is in the same hemisphere
  // as the surface normal, so that TIR is handled correctly.
  if (glm::dot(ray_half, normal) < 0.0f) {
    ray_half = -ray_half;
  }
  glm::vec3 F = fresnel_->Evaluate(glm::dot(ray_in_reverse, ray_half));
  return base_color_ * distribution_->D(ray_half) * distribution_->G(ray_out, ray_in_reverse) * F /
    (4 * cos_theta_in * cos_theta_out);
}

glm::vec3 MicrofacetReflection::SampleRayIn(const glm::vec3& normal, const glm::vec3& ray_out, glm::vec3* ray_in, float* pdf, std::mt19937& rng, bool is_front_face) const
{
  // Sample microfacet orientation $\wh$ and reflected direction $\wi$
  if (glm::dot(normal, ray_out) == 0) return glm::vec3{ 0.0f };
  glm::vec3 ray_half = distribution_->SampleWh(ray_out, rng);
  if (glm::dot(ray_out, ray_half) < 0) return glm::vec3{ 0.0f };   // Should be rare
  *ray_in = glm::reflect(ray_out, ray_half);
  //if (!SameHemisphere(wo, *wi)) return Spectrum(0.f);

  //// Compute PDF of _wi_ for microfacet reflection
  //*pdf = distribution->Pdf(wo, ray_half) / (4 * Dot(wo, ray_half));
  *pdf = GetPdf(normal, ray_out, *ray_in, is_front_face);
  return GetBsdf(normal, ray_out, *ray_in, is_front_face);
}

//glm::vec3 MicrofacetReflection::World2Tangent_(const glm::vec3& normal, const glm::vec3& w)
//{
//  return glm::vec3();
//}

float MicrofacetTransmission::GetPdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const
{
  glm::vec3 out_normal; // normal pointing outwards
  if (is_front_face) {
    out_normal = normal;
  }
  else {
    out_normal = -normal;
  }
  glm::vec3 ray_in_reverse = -ray_in;
  if (glm::dot(out_normal, ray_in_reverse) * glm::dot(out_normal, ray_out) > 0.0f) {
    //LAND_INFO("cos_out {}, cos_in {}", glm::dot(out_normal, ray_in_reverse), glm::dot(out_normal, ray_out));
    return 0.0f;
  }
  float cos_theta_out = glm::dot(out_normal, ray_out);

  float ior = cos_theta_out > 0 ? (ior_b_ / ior_a_) : (ior_a_ / ior_b_);
  glm::vec3 ray_half = glm::normalize(ray_in_reverse * ior + ray_out);
  // Handle degenerate cases for microfacet reflection

  //if (glm::dot(ray_out, ray_half) * glm::dot(ray_in_reverse, ray_half) > 0) return 0.0f;
  float sqrt_denom = glm::dot(ray_out, ray_half) + ior * glm::dot(ray_in_reverse, ray_half);
  float d_ray_half_d_ray_in_rev =
    std::abs((ior * ior * glm::dot(ray_in_reverse, ray_half)) / (sqrt_denom * sqrt_denom));
  //LAND_INFO("sqrt_denom {}, d_ray_half_d_ray_in_rev {}", sqrt_denom, d_ray_half_d_ray_in_rev);
  return distribution_->Pdf(ray_out, ray_half) * d_ray_half_d_ray_in_rev;
}

glm::vec3 MicrofacetTransmission::GetBsdf(const glm::vec3& normal, const glm::vec3& ray_out, const glm::vec3& ray_in, bool is_front_face) const
{
  glm::vec3 out_normal; // normal pointing outwards
  if (is_front_face) {
    out_normal = normal;
  }
  else {
    out_normal = -normal;
  }
  glm::vec3 ray_in_reverse = -ray_in;
  float cos_theta_out = glm::dot(out_normal, ray_out);
  float cos_theta_in = glm::dot(out_normal, ray_in_reverse);

  if (cos_theta_in == 0 || cos_theta_out == 0) return glm::vec3{ 0.0f };

  float ior  = cos_theta_out > 0 ? (ior_b_ / ior_a_) : (ior_a_ / ior_b_);
  glm::vec3 ray_half = ray_in_reverse * ior + ray_out;
  // Handle degenerate cases for microfacet reflection

  if (glm::all(glm::equal(ray_half, glm::vec3{ 0.0f }))) return glm::vec3{ 0.0f };
  ray_half = glm::normalize(ray_half);
  // For the Fresnel call, make sure that ray_half is in the same hemisphere
  // as the surface normal, so that TIR is handled correctly.
  if (glm::dot(ray_half, out_normal) < 0.0f) {
    ray_half = -ray_half;
  }
  if (glm::dot(ray_out, ray_half) * glm::dot(ray_in_reverse, ray_half) > 0) return glm::vec3{ 0.0f };
  glm::vec3 F = fresnel_.Evaluate(glm::dot(ray_out, ray_half));
  float sqrt_denom = glm::dot(ray_out, ray_half) + ior * glm::dot(ray_in_reverse, ray_half);
  float factor = 1 / ior;

  return (1.0f - F) * base_color_ *
    glm::abs(distribution_->D(ray_half) * distribution_->G(ray_out, ray_in_reverse) * ior * ior *
      glm::abs(glm::dot(ray_in_reverse, ray_half)) * glm::abs(glm::dot(ray_out, ray_half)) * factor * factor /
      (cos_theta_in * cos_theta_out * sqrt_denom * sqrt_denom));
}

glm::vec3 MicrofacetTransmission::SampleRayIn(const glm::vec3& normal, const glm::vec3& ray_out, glm::vec3* ray_in, float* pdf, std::mt19937& rng, bool is_front_face) const
{
  //glm::vec3 out_normal; // normal pointing outwards
  //if (is_front_face) {
  //  out_normal = normal;
  //}
  //else {
  //  out_normal = -normal;
  //}
  // Sample microfacet orientation $\wh$ and reflected direction $\wi$
  if (glm::dot(normal, ray_out) == 0) return glm::vec3{ 0.0f };
  glm::vec3 ray_half = distribution_->SampleWh(ray_out, rng);
  if (glm::abs(glm::length(ray_half) - 1.0f) > 1e-5f) {
    LAND_ERROR("Unnormalized ray half! {}", glm::length(ray_half));
  }
  if (glm::dot(ray_out, ray_half) < 0) return glm::vec3{ 0.0f };   // Should be rare

  float cos_theta_out = glm::dot(normal, ray_out);
  float ior = is_front_face ? (ior_b_ / ior_a_) : (ior_a_ / ior_b_);
  *ray_in = -glm::refract(-ray_out, ray_half, 1.0f/ ior);
  //*ray_in = ray_out;
  if (glm::length(*ray_in) < 1e-5f) {
    return glm::vec3{ 0.0f };
  }
  *pdf = GetPdf(normal, ray_out, *ray_in, is_front_face);
  auto bsdf = GetBsdf(normal, ray_out, *ray_in, is_front_face);
  //if (*pdf > 0.0f) {
  //  LAND_INFO("Normal {}, Ray out {}, is_front_face {}. Sampled ray in {}, pdf {}, bsdf {}",
  //    glm::to_string(normal),
  //    glm::to_string(ray_out),
  //    is_front_face,
  //    glm::to_string(*ray_in),
  //    *pdf,
  //    glm::to_string(bsdf));
  //}
  return bsdf;
}


} //namespace sparks