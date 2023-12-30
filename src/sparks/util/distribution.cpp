// https://github.com/mmp/pbrt-v3/
#include "distribution.h"
#include "glm/gtx/string_cast.hpp"

namespace sparks {
float MicrofacetDistribution::D(const glm::vec3& wh) const
{
  glm::vec3 tangent_wh = GetWorld2Tangent_() * wh;
  if (CosTheta(tangent_wh) == 0.0f) {
    return 0.0f;
  }
  float tan2Theta = Tan2Theta(tangent_wh);
  const float cos4Theta = Cos2Theta(tangent_wh) * Cos2Theta(tangent_wh);
  float e =
    (Cos2Phi(tangent_wh) / (alpha_x_ * alpha_x_) + Sin2Phi(tangent_wh) / (alpha_y_ * alpha_y_)) *
    tan2Theta;
  return 1 / (PI * alpha_x_ * alpha_y_ * cos4Theta * (1 + e) * (1 + e));
}
float MicrofacetDistribution::Lambda(const glm::vec3& w) const
{
  glm::vec3 tangent_w = GetWorld2Tangent_() * w; // convert to tangent space
  if (CosTheta(tangent_w) == 0.0f) {
    return 0.0f;
  }
  float absTanTheta = glm::abs(TanTheta(tangent_w));

  // Compute _alpha_ for direction _w_
  float alpha =
    std::sqrt(Cos2Phi(tangent_w) * alpha_x_ * alpha_x_ + Sin2Phi(tangent_w) * alpha_y_ * alpha_y_);
  float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
  return (-1 + std::sqrt(1.0f + alpha2Tan2Theta)) / 2;
}
glm::vec3 MicrofacetDistribution::SampleWh(const glm::vec3& wo, std::mt19937& rng) const
{
  glm::vec3 tangent_wo = GetWorld2Tangent_() * wo;
  if (tangent_wo.z == 0.0f) {
    LAND_ERROR("Zero tangent wo! wo {}, normal {}", glm::to_string(wo), glm::to_string(normal_));
  }
  bool flip = tangent_wo.z < 0;
  if (flip) {
    tangent_wo = -tangent_wo;
  }
  //wh = TrowbridgeReitzSample(tangent_wo, alpha_x_, alpha_y_, u[0], u[1]);
      // 1. stretch tangent_wo
  glm::vec3 tangent_wo_stretched =
    glm::normalize(glm::vec3(alpha_x_ * tangent_wo.x, alpha_y_ * tangent_wo.y, tangent_wo.z));

  // 2. simulate P22_{tangent_wo}(x_slope, y_slope, 1, 1)
  float u1 = std::uniform_real_distribution<float>(0.0f, 1.0f)(rng);
  float u2 = std::uniform_real_distribution<float>(0.0f, 1.0f)(rng);
  float slope_x, slope_y;
  TrowbridgeReitzSample11_(CosTheta(tangent_wo_stretched), u1, u2, &slope_x, &slope_y);

  // 3. rotate
  float tmp = CosPhi(tangent_wo_stretched) * slope_x - SinPhi(tangent_wo_stretched) * slope_y;
  slope_y = SinPhi(tangent_wo_stretched) * slope_x + CosPhi(tangent_wo_stretched) * slope_y;
  slope_x = tmp;

  // 4. unstretch
  slope_x = alpha_x_ * slope_x;
  slope_y = alpha_y_ * slope_y;

  // 5. compute normal
  glm::vec3 tangent_wh =  glm::normalize(glm::vec3{ -slope_x, -slope_y, 1.0f });
  if (flip) {
    tangent_wh = -tangent_wh;
  }
  
  auto result = GetTangent2World_() * tangent_wh;
  if (glm::abs(glm::length(result) - 1.0f) > 1e-5f) {
    LAND_ERROR("Unnormalized result! {}, tangent_wh {}, tangent2world_ {}", glm::length(result), glm::to_string(tangent_wh), glm::to_string(GetTangent2World_()));
  }
  return GetTangent2World_() * tangent_wh;
}
float MicrofacetDistribution::Pdf(const glm::vec3& wo, const glm::vec3& wh) const
{
	return D(wh) * G1(wo) * glm::abs(glm::dot(wo, wh)) / glm::abs(glm::dot(wo, normal_));
}

void MicrofacetDistribution::TrowbridgeReitzSample11_(float cosTheta, float u1, float u2, float* slope_x, float* slope_y) const
{    // special case (normal incidence)
  if (cosTheta > .9999) {
    float r = sqrt(u1 / (1 - u1));
    float phi = 6.28318530718 * u2;
    *slope_x = r * glm::cos(phi);
    *slope_y = r * glm::sin(phi);
    return;
  }

  if (cosTheta == 0.0f) {
    LAND_WARN("zero cosTheta!");
  }

  float sinTheta =
    glm::sqrt(glm::max((float)0, (float)1 - cosTheta * cosTheta));
  float tanTheta = sinTheta / cosTheta;
  float a = 1 / tanTheta;
  float G1 = 2 / (1 + glm::sqrt(1.0f + 1.0f / (a * a)));

  // sample slope_x
  float A = 2 * u1 / G1 - 1;
  float tmp = 1.f / (A * A - 1.f);
  if (tmp > 1e10) tmp = 1e10;
  float B = tanTheta;
  float D = glm::sqrt(
    glm::max(float(B * B * tmp * tmp - (A * A - B * B) * tmp), float(0)));
  float slope_x_1 = B * tmp - D;
  float slope_x_2 = B * tmp + D;
  *slope_x = (A < 0 || slope_x_2 > 1.0f / tanTheta) ? slope_x_1 : slope_x_2;

  // sample slope_y
  float S;
  if (u2 > 0.5f) {
    S = 1.f;
    u2 = 2.f * (u2 - .5f);
  }
  else {
    S = -1.f;
    u2 = 2.f * (.5f - u2);
  }
  float z =
    (u2 * (u2 * (u2 * 0.27385f - 0.73369f) + 0.46341f)) /
    (u2 * (u2 * (u2 * 0.093073f + 0.309420f) - 1.000000f) + 0.597999f);
  *slope_y = S * z * glm::sqrt(1.0f + *slope_x * *slope_x);

  //CHECK(!std::isinf(*slope_y));
  //CHECK(!std::isnan(*slope_y));
}

glm::mat3 MicrofacetDistribution::GetTangent2World_() const
{
  glm::vec3 l{ normal_.y, -normal_.x, 0.0f };
  glm::vec3 m{
    normal_.x * normal_.z,
    normal_.y * normal_.z,
    -normal_.x * normal_.x - normal_.y * normal_.y };
  l = glm::normalize(l);
  m = glm::normalize(m);
  if (glm::abs(glm::length(l) - 1.0f) > 1e-5f) {
    LAND_ERROR("Unnormalized l! {}", glm::to_string(l));
  }
  if (glm::abs(glm::length(m) - 1.0f) > 1e-5f) {
    LAND_ERROR("Unnormalized m! {}", glm::to_string(m));
  }
  if (glm::abs(glm::length(normal_) - 1.0f) > 1e-5f) {
    LAND_ERROR("Unnormalized normal! {}", glm::to_string(normal_));
  }
  return glm::mat3{ m,l,normal_ };
}

glm::mat3 MicrofacetDistribution::GetWorld2Tangent_() const
{
  return glm::inverse(GetTangent2World_());
};
} // namespace sparks

