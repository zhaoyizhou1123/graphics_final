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
  glm::vec3 albedo_color{0.8f};
  int albedo_texture_id{0};
  glm::vec3 emission{0.0f};
  float emission_strength{1.0f};
  float alpha{1.0f};
  float ior{ 1.0f }; // index of refraction, default 1 (air)
  MaterialType material_type{MATERIAL_TYPE_LAMBERTIAN};
  //int height_texture_id{ 1 };
  //float reserve[1]{};
  int normal_texture_id{ 1 };
  Material() = default;
  explicit Material(const glm::vec3 &albedo);
  Material(Scene *scene, const tinyxml2::XMLElement *material_element);

  glm::vec3 GetBrdf(
    const glm::vec3& pos,
    const glm::vec3& ray_in,
    const glm::vec3& ray_out);

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
