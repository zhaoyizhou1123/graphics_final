#pragma once
#include "cstdint"
#include "glm/glm.hpp"
#include "sparks/assets/util.h"

namespace sparks {

enum MaterialType : int {
  MATERIAL_TYPE_LAMBERTIAN = 0,
  MATERIAL_TYPE_SPECULAR = 1,
  MATERIAL_TYPE_TRANSMISSIVE = 2,
  MATERIAL_TYPE_PRINCIPLED = 3,
  MATERIAL_TYPE_EMISSION = 4
};

class Scene;

class Material {
public:
  int albedo_texture_id{ 0 };
  int normal_texture_id{ 1 };
  MaterialType material_type{ MATERIAL_TYPE_LAMBERTIAN };
  float padding1{0.0f};
  // Principled BSDF parameters
  glm::vec3 albedo_color{0.8f};
  float metallic{ 0.0f };
  float ior{ 1.0f }; // index of refraction, default 1 (air)
  float roughness{ 0.0f };
  float spec_trans{ 0.0f };
  bool thin{ true };
  bool padding2[3]{};
  float flatness{ 0.0f };
  float diff_trans{ 0.0f }; // 0: all diffuse is reflected -> 1, transmitted
  float padding3[2]{};

  glm::vec3 emission{0.0f};
  float emission_strength{1.0f};
  float alpha{1.0f};
  //int height_texture_id{ 1 };
  float padding4[3]{};

  Material() = default;
  explicit Material(const glm::vec3 &albedo);
  Material(Scene *scene, const tinyxml2::XMLElement *material_element);

  // Return pdf
  float GetBsdf(
    const glm::vec3& pos,
    const glm::vec3& ray_in,
    const glm::vec3& ray_out) const;

  // Sample ray_in given ray_out and record pdf
  void SampleRayIn(
    const glm::vec3& pos,
    glm::vec3* ray_in,
    const glm::vec3& ray_out,
    float* pdf) const;

  /* Compute reflection and refaction light
  * @param is_front: True if incident ray is in air, and want to enter the material
  * @param normal: Not guaranteed to be in the correct direction.
  * @return: fresnel term in [0,1]. If 1, total reflection, then refaction will be (0,0,0).
  */
  //float GetReflectRefract(
  //  const glm::vec3& dir_in,
  //  const glm::vec3& normal,
  //  bool is_front,
  //  glm::vec3& dir_reflect,
  //  glm::vec3& dir_refract
  //);
private:
  /* Compute refraction angle cos and sin.
  * @param is_front: True if incident ray is in air, and want to enter the material
  * @param normal: Not guaranteed to be in the correct direction.
  * @return: True if refraction exists. If false, cos_thetat and sin_thetat are meaningless
  */ 
  bool GetTransAngle_(const glm::vec3& dir_in, const glm::vec3& normal, bool is_front,
                      float* cos_thetat, float* sin_thetat);
  float GetFresnel_(const glm::vec3& dir_in);
  glm::vec3 GetRefractDir_(const glm::vec3& dir_in, const glm::vec3& normal, bool is_front);
};
}  // namespace sparks
