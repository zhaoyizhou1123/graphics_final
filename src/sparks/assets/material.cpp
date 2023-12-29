#include "sparks/assets/material.h"

#include "grassland/grassland.h"
#include "sparks/assets/scene.h"
#include "sparks/assets/texture.h"
#include "sparks/util/util.h"

namespace sparks {

namespace {
std::unordered_map<std::string, MaterialType> material_name_map{
    {"lambertian", MATERIAL_TYPE_LAMBERTIAN},
    {"specular", MATERIAL_TYPE_SPECULAR},
    {"transmissive", MATERIAL_TYPE_TRANSMISSIVE},
    {"principled", MATERIAL_TYPE_PRINCIPLED},
    {"emission", MATERIAL_TYPE_EMISSION}};
}

Material::Material(Scene *scene, const tinyxml2::XMLElement *material_element)
    : Material() {
  if (!material_element) {
    return;
  }

  albedo_color = glm::vec3{1.0f};

  auto child_element = material_element->FirstChildElement("albedo");
  if (child_element) {
    albedo_color = StringToVec3(child_element->FindAttribute("value")->Value());
  }

  child_element = material_element->FirstChildElement("albedo_texture");
  if (child_element) {
    std::string path = child_element->FindAttribute("value")->Value();
    Texture albedo_texture(1, 1);
    if (Texture::Load(path, albedo_texture)) {
      albedo_texture_id =
          scene->AddTexture(albedo_texture, PathToFilename(path));
      LAND_INFO("Loaded texture from {}", path);
    }
  }

  child_element = material_element->FirstChildElement("normal_texture");
  if (child_element) {
    std::string path = child_element->FindAttribute("value")->Value();
    Texture normal_texture(1, 1);
    if (Texture::Load(path, normal_texture)) {
      normal_texture_id =
        scene->AddTexture(normal_texture, PathToFilename(path));
      LAND_INFO("Loaded normal texture from {}", path);
    }
  }

  //child_element = material_element->FirstChildElement("height_texture");
  //if (child_element) {
  //  std::string path = child_element->FindAttribute("value")->Value();
  //  Texture height_texture(1, 1);
  //  if (Texture::Load(path, height_texture)) {
  //    height_texture_id =
  //      scene->AddTexture(height_texture, PathToFilename(path));
  //    LAND_INFO("Loaded height texture from {}", path);
  //  }
  //}

  child_element = material_element->FirstChildElement("emission");
  if (child_element) {
    emission = StringToVec3(child_element->FindAttribute("value")->Value());
  }

  child_element = material_element->FirstChildElement("emission_strength");
  if (child_element) {
    emission_strength =
        std::stof(child_element->FindAttribute("value")->Value());
  }

  child_element = material_element->FirstChildElement("alpha");
  if (child_element) {
    alpha = std::stof(child_element->FindAttribute("value")->Value());
  }

  child_element = material_element->FirstChildElement("ior");
  if (child_element) {
    alpha = std::stof(child_element->FindAttribute("value")->Value());
  }

  material_type =
      material_name_map[material_element->FindAttribute("type")->Value()];
}

Material::Material(const glm::vec3 &albedo) : Material() {
  albedo_color = albedo;
}

float Material::GetBsdf(
    const glm::vec3& pos,
    const glm::vec3& ray_in,
    const glm::vec3& ray_out) const {
  return 0.0f;
}

void Material::SampleRayIn(const glm::vec3& pos, glm::vec3* ray_in, const glm::vec3& ray_out, float* pdf) const
{
}

//float Material::GetReflectRefract(const glm::vec3& dir_in, const glm::vec3& normal, bool is_front, glm::vec3& dir_reflect, glm::vec3& dir_refract)
//{
//  if (glm::abs(glm::length(dir_in) - 1.0f) > 1e-3) {
//    LAND_ERROR("Unnormalized in direction!");
//  }
//  if (glm::abs(glm::length(normal) - 1.0f) > 1e-3) {
//    LAND_ERROR("Unnormalized normal!");
//  }
//  // Get true normal direction and theta_i
//  glm::vec3 incident_normal = normal; // normal at incident direction
//  float cos_thetai = glm::dot(dir_in, normal);
//  if (cos_thetai < 0) { // incident_normal direction is correct, modify cos_thetai
//    cos_thetai = -cos_thetai;
//  }
//  else { // reverse incident_normal direction
//    incident_normal = -normal;
//  }
//  float square_sin_thetai = 1 - cos_thetai * cos_thetai;
//
//  // Get relection direction
//  dir_reflect = glm::reflect(dir_in, normal);
//
//  // Get relative ior, in/refract
//  float ior_in_over_refract = 1.0f / ior; // eta_i / eta_t
//  if (!is_front) {
//    ior_in_over_refract = ior;
//  }
//  float square_cos_thetat = 1 - ior_in_over_refract * ior_in_over_refract * square_sin_thetai;
//
//  if (square_cos_thetat)
//}
}  // namespace sparks
