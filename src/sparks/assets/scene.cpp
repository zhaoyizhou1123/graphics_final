﻿#include "sparks/assets/scene.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include "sparks/assets/accelerated_mesh.h"
#include "sparks/util/util.h"
#include "sparks/geometries/plane.h"
#include <memory>
#include <numeric>
#include <glm/gtx/string_cast.hpp>

namespace sparks {

Scene::Scene() {
  AddTexture(Texture(1, 1, glm::vec4{1.0f}, SAMPLE_TYPE_LINEAR), "Pure White");
  AddTexture(Texture(1, 1, glm::vec4{0.0f}, SAMPLE_TYPE_LINEAR), "Pure Black");
}

int Scene::AddTexture(const Texture &texture, const std::string &name) {
  textures_.push_back(texture);
  texture_names_.push_back(name);
  return int(textures_.size() - 1);
}

const std::vector<Texture> &Scene::GetTextures() const {
  return textures_;
}

int Scene::GetTextureCount() const {
  return int(textures_.size());
}

void Scene::Clear() {
  textures_.clear();
  entities_.clear();
  camera_ = Camera{};
}

Entity &Scene::GetEntity(int entity_index) {
  return entities_[entity_index];
}

const Entity &Scene::GetEntity(int entity_index) const {
  return entities_[entity_index];
}

std::vector<Entity> &Scene::GetEntities() {
  return entities_;
}

const std::vector<Entity> &Scene::GetEntities() const {
  return entities_;
}

int Scene::GetEntityCount() const {
  return int(entities_.size());
}

Lights& Scene::GetLights()
{
  return lights_;
}

const Lights& Scene::GetLights() const
{
  return lights_;
}

Camera &Scene::GetCamera() {
  return camera_;
}

const Camera &Scene::GetCamera() const {
  return camera_;
}

void Scene::SetCamera(const Camera &camera) {
  camera_ = camera;
}

glm::mat4 Scene::GetCameraToWorld() const {
  return glm::translate(glm::mat4{1.0f}, camera_position_) *
         ComposeRotation(camera_pitch_yaw_roll_) *
         glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f, 1.0f, 1.0f});
}

void Scene::SetCameraToWorld(const glm::mat4 &camera_to_world) {
  camera_pitch_yaw_roll_ = DecomposeRotation(
      camera_to_world *
      glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f, 1.0f, 1.0f}));
  camera_position_ = camera_to_world[3];
}

int &Scene::GetEnvmapId() {
  return envmap_id_;
}

const int &Scene::GetEnvmapId() const {
  return envmap_id_;
}

float &Scene::GetEnvmapOffset() {
  return envmap_offset_;
}

const float &Scene::GetEnvmapOffset() const {
  return envmap_offset_;
}

glm::vec3 &Scene::GetCameraPosition() {
  return camera_position_;
}
const glm::vec3 &Scene::GetCameraPosition() const {
  return camera_position_;
}
float &Scene::GetCameraSpeed() {
  return camera_speed_;
}
const float &Scene::GetCameraSpeed() const {
  return camera_speed_;
}
glm::vec3 &Scene::GetCameraPitchYawRoll() {
  return camera_pitch_yaw_roll_;
}
const glm::vec3 &Scene::GetCameraPitchYawRoll() const {
  return camera_pitch_yaw_roll_;
}

void Scene::UpdateEnvmapConfiguration() {
  const auto &scene = *this;
  auto envmap_id = scene.GetEnvmapId();
  auto &envmap_texture = scene.GetTextures()[envmap_id]; // Get current texture
  auto buffer = envmap_texture.GetBuffer();

  envmap_minor_color_ = glm::vec3{0.0f};
  envmap_major_color_ = glm::vec3{0.0f};
  envmap_cdf_.resize(envmap_texture.GetWidth() * envmap_texture.GetHeight());

  std::vector<float> sample_scale_(envmap_texture.GetHeight() + 1);
  auto inv_width = 1.0f / float(envmap_texture.GetWidth());
  auto inv_height = 1.0f / float(envmap_texture.GetHeight());
  for (int i = 0; i <= envmap_texture.GetHeight(); i++) {
    float x = float(i) * glm::pi<float>() * inv_height; // \pi*i/h
    sample_scale_[i] = (-std::cos(x) + 1.0f) * 0.5f;
  }

  auto width_height = envmap_texture.GetWidth() * envmap_texture.GetHeight();
  float total_weight = 0.0f;
  float major_strength = -1.0f;
  for (int y = 0; y < envmap_texture.GetHeight(); y++) {
    auto scale = sample_scale_[y + 1] - sample_scale_[y];

    auto theta = (float(y) + 0.5f) * inv_height * glm::pi<float>(); // sphere coordinate theta
    auto sin_theta = std::sin(theta);
    auto cos_theta = std::cos(theta);

    for (int x = 0; x < envmap_texture.GetWidth(); x++) {
      auto phi = (float(x) + 0.5f) * inv_width * glm::pi<float>() * 2.0f;
      auto sin_phi = std::sin(phi);
      auto cos_phi = std::cos(phi);

      auto i = y * envmap_texture.GetWidth() + x;
      auto color = glm::vec3{buffer[i]};
      auto minor_color = glm::clamp(color, 0.0f, 1.0f);
      auto major_color = color - minor_color;
      envmap_major_color_ += major_color * (scale * inv_width);
      envmap_minor_color_ += minor_color * (scale * inv_width);
      color *= scale;

      auto strength = std::max(color.x, std::max(color.y, color.z));
      if (strength > major_strength) {
        envmap_light_direction_ = {sin_theta * sin_phi, cos_theta,
                                   -sin_theta * cos_phi}; // Coordinate of sphere?
        major_strength = strength;
      }

      total_weight += strength * scale;
      envmap_cdf_[i] = total_weight;
    }
  }

  auto inv_total_weight = 1.0f / total_weight;
  for (auto &v : envmap_cdf_) {
    v *= inv_total_weight;
  }
}

// Return a fixed light direction in the scene
glm::vec3 Scene::GetEnvmapLightDirection() const {
  float sin_offset = std::sin(envmap_offset_);
  float cos_offset = std::cos(envmap_offset_);

  // Can simply think as returning envmap_light_direction_, only updated during UpdateEnvmapConfiguration.
  return {cos_offset * envmap_light_direction_.x +
              sin_offset * envmap_light_direction_.z,
          envmap_light_direction_.y,
          -sin_offset * envmap_light_direction_.x +
              cos_offset * envmap_light_direction_.z};
}
const glm::vec3 &Scene::GetEnvmapMinorColor() const {
  return envmap_minor_color_;
}
const glm::vec3 &Scene::GetEnvmapMajorColor() const {
  return envmap_major_color_;
}
const std::vector<float> &Scene::GetEnvmapCdf() const {
  return envmap_cdf_;
}

float Scene::TraceRay(const glm::vec3 &origin,
                      const glm::vec3 &direction,
                      float t_min,
                      float t_max,
                      HitRecord *hit_record) const {
  const std::vector<int>& entities_order_list = RayTracingPreSort_(
    origin,
    direction,
    t_min,
    t_max);
  //LAND_INFO("Entity list size {}", entities_order_list.size());
  float result = -1.0f;
  HitRecord local_hit_record;
  float local_result;
  // Iterate through entities_order_list elements
  for (auto itr = entities_order_list.begin(); itr != entities_order_list.end(); itr++) {
    int entity_id = *itr; // current entity index
    auto &entity = entities_[entity_id];
    auto &transform = entity.GetTransformMatrix();
    auto inv_transform = glm::inverse(transform);
    auto transformed_direction =
        glm::vec3{inv_transform * glm::vec4{direction, 0.0f}};
    auto transformed_direction_length = glm::length(transformed_direction);
    if (transformed_direction_length < 1e-6) {
      continue;
    }
    // Improvement, use result in place of t_min, when a valid result already exists
    local_result = entity.GetModel()->TraceRayImprove(
        inv_transform * glm::vec4{origin, 1.0f},
        transformed_direction / transformed_direction_length, t_min * transformed_direction_length,
        result * transformed_direction_length, hit_record ? &local_hit_record : nullptr);
    local_result /= transformed_direction_length;
    if (local_result > t_min && local_result < t_max &&
        (result < 0.0f || local_result < result)) {
      result = local_result;
      if (hit_record) {
        local_hit_record.position =
            transform * glm::vec4{local_hit_record.position, 1.0f};
        local_hit_record.normal = glm::transpose(inv_transform) *
                                  glm::vec4{local_hit_record.normal, 0.0f};
        local_hit_record.tangent =
            transform * glm::vec4{local_hit_record.tangent, 0.0f};
        local_hit_record.geometry_normal =
            glm::transpose(inv_transform) *
            glm::vec4{local_hit_record.geometry_normal, 0.0f};
        *hit_record = local_hit_record;
        hit_record->hit_entity_id = entity_id;
      }
    }
  }
  if (hit_record) {
    hit_record->geometry_normal = glm::normalize(hit_record->geometry_normal);
    hit_record->normal = glm::normalize(hit_record->normal);
    hit_record->tangent = glm::normalize(hit_record->tangent);
  }
  return result;
}

float Scene::TraceRay(const glm::vec3& origin,
  const glm::vec3& direction,
  float time,
  float t_min,
  float t_max,
  HitRecord* hit_record) const {
  //// TODO: Consider motion blur in presort
  //const std::vector<int>& entities_order_list = RayTracingPreSort_(
  //  origin,
  //  direction,
  //  time,
  //  t_min,
  //  t_max);
  const std::vector<int>& entities_order_list = RayTracingPreSort_(
    origin,
    direction,
    time,
    t_min,
    t_max);
  //LAND_INFO("Entity list size {}", entities_order_list.size());
  float result = -1.0f;
  HitRecord local_hit_record;
  float local_result;
  // Iterate through entities_order_list elements
  for (auto itr = entities_order_list.begin(); itr != entities_order_list.end(); itr++) {
    int entity_id = *itr; // current entity index
    auto& entity = entities_[entity_id];
    auto &transform = entity.GetTransformMatrix(time);
    //auto& transform = entity.GetTransformMatrix();
    //glm::mat4 transform = entity.GetTransformMatrix(0.0f);
    auto inv_transform = glm::inverse(transform);
    auto transformed_direction =
      glm::vec3{ inv_transform * glm::vec4{direction, 0.0f} };
    auto transformed_direction_length = glm::length(transformed_direction);
    if (transformed_direction_length < 1e-6) {
      continue;
    }
    // Improvement, use result in place of t_min, when a valid result already exists
    local_result = entity.GetModel()->TraceRayImprove(
      inv_transform * glm::vec4{ origin, 1.0f },
      transformed_direction / transformed_direction_length, t_min * transformed_direction_length,
      result * transformed_direction_length, hit_record ? &local_hit_record : nullptr);
    local_result /= transformed_direction_length;
    if (local_result > t_min && local_result < t_max &&
      (result < 0.0f || local_result < result)) {
      result = local_result;
      if (hit_record) {
        local_hit_record.position =
          transform * glm::vec4{ local_hit_record.position, 1.0f };
        local_hit_record.normal = glm::transpose(inv_transform) *
          glm::vec4{ local_hit_record.normal, 0.0f };
        local_hit_record.tangent =
          transform * glm::vec4{ local_hit_record.tangent, 0.0f };
        local_hit_record.geometry_normal =
          glm::transpose(inv_transform) *
          glm::vec4{ local_hit_record.geometry_normal, 0.0f };
        *hit_record = local_hit_record;
        hit_record->hit_entity_id = entity_id;
      }
    }
  }
  if (hit_record) {
    hit_record->geometry_normal = glm::normalize(hit_record->geometry_normal);
    hit_record->normal = glm::normalize(hit_record->normal);
    hit_record->tangent = glm::normalize(hit_record->tangent);
  }
  return result;
}

glm::vec4 Scene::SampleEnvmap(const glm::vec3 &direction) const {
  float x = envmap_offset_;
  float y = acos(direction.y) * INV_PI;
  if (glm::length(glm::vec2{direction.x, direction.y}) > 1e-4) {
    x += glm::atan(direction.x, -direction.z);
  }
  x *= INV_PI * 0.5;
  return textures_[envmap_id_].Sample(glm::vec2{x, y});
}

const Texture &Scene::GetTexture(int texture_id) const {
  return textures_[texture_id];
}

std::vector<const char *> Scene::GetTextureNameList() const {
  std::vector<const char *> result;
  result.reserve(texture_names_.size());
  for (const auto &texture_name : texture_names_) {
    result.push_back(texture_name.data());
  }
  return result;
}

std::vector<const char *> Scene::GetEntityNameList() const {
  std::vector<const char *> result;
  result.reserve(entities_.size());
  for (const auto &entity : entities_) {
    result.push_back(entity.GetName().data());
  }
  return result;
}

bool Scene::TextureCombo(const char *label, int *current_item) const {
  return ImGui::Combo(label, current_item, GetTextureNameList().data(),
                      textures_.size());
}

bool Scene::EntityCombo(const char *label, int *current_item) const {
  return ImGui::Combo(label, current_item, GetEntityNameList().data(),
                      entities_.size());
}

int Scene::LoadTexture(const std::string &file_path) {
  Texture texture;
  if (Texture::Load(file_path, texture)) {
    return AddTexture(texture, PathToFilename(file_path));
  } else {
    LAND_WARN("[Sparks] Load Texture \"{}\" failed.", file_path);
    return 0;
  }
}

int Scene::LoadObjMesh(const std::string &file_path) {
  //AcceleratedMesh mesh;
  Mesh mesh;
  if (Mesh::LoadObjFile(file_path, mesh)) {
    //auto acc_mesh = AcceleratedMesh(mesh);
    //mesh.BuildAccelerationStructure();
    return AddEntity(std::make_unique<AcceleratedMesh>(mesh), Material{}, glm::mat4{1.0f},
                     PathToFilename(file_path));
  } else {
    return -1;
  }
  return -1;
}

Scene::Scene(const std::string& filename) : Scene() {
  auto doc = std::make_unique<tinyxml2::XMLDocument>();
  doc->LoadFile(filename.c_str());
  tinyxml2::XMLElement* rootElement = doc->RootElement();

  glm::mat4 camera_to_world = glm::inverse(
      glm::lookAt(glm::vec3{ 2.0f, 1.0f, 3.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f },
          glm::vec3{ 0.0f, 1.0f, 0.0f }));

  for (tinyxml2::XMLElement* child_element = rootElement->FirstChildElement();
    child_element; child_element = child_element->NextSiblingElement()) {
    // child_element: each object
    std::string element_type{ child_element->Value() }; // type of object
    if (element_type == "envmap") {
      std::string envmap_type = child_element->FindAttribute("type")->Value();
      if (envmap_type == "file") {
        std::string envmap_filename =
          child_element->FindAttribute("value")->Value();
        Texture envmap;
        Texture::Load(envmap_filename, envmap);
        envmap_id_ = AddTexture(envmap, PathToFilename(envmap_filename)); // Update envmap_id_ to the last texture index
      }
      else if (envmap_type == "color") {
        glm::vec3 color =
          StringToVec3(child_element->FindAttribute("value")->Value());
        Texture envmap(1, 1, glm::vec4{ color, 1.0f });
        envmap_id_ = AddTexture(envmap, "Environment Map");
      }
    }
    else if (element_type == "camera") {
      camera_to_world =
        XmlTransformMatrix(child_element->FirstChildElement("transform"));
      float fov = 60.0f;
      float aperture = 0.0f;
      float focal_length = 3.0f;
      float shutter = 0.0f;
      auto grandchild_element = child_element->FirstChildElement("fov");
      if (grandchild_element) {
        fov = std::stof(grandchild_element->FindAttribute("value")->Value());
      }
      grandchild_element = child_element->FirstChildElement("speed");
      if (grandchild_element) {
        camera_speed_ =
          std::stof(grandchild_element->FindAttribute("value")->Value());
      }
      grandchild_element = child_element->FirstChildElement("aperture");
      if (grandchild_element) {
        aperture =
          std::stof(grandchild_element->FindAttribute("value")->Value());
      }
      grandchild_element = child_element->FirstChildElement("focal_length");
      if (grandchild_element) {
        focal_length =
          std::stof(grandchild_element->FindAttribute("value")->Value());
      }
      grandchild_element = child_element->FirstChildElement("shutter");
      if (grandchild_element) {
        shutter =
          std::stof(grandchild_element->FindAttribute("value")->Value());
      }
      camera_ = Camera(shutter, fov, aperture, focal_length);
      LAND_INFO("Loaded camera");
    }
    else if (element_type == "model") {
      Mesh mesh = Mesh(child_element);
      //LAND_INFO("Builded mesh");
      Material material{};

      glm::vec3 speed{ 0.0f };
      auto speed_element = child_element->FirstChildElement("speed");
      if (speed_element) {
        speed = StringToVec3(speed_element->FindAttribute("value")->Value());
      }

      auto grandchild_element = child_element->FirstChildElement("material");
      if (grandchild_element) {
        material = Material(this, grandchild_element);
        std::string material_type{ grandchild_element->FindAttribute("type")->Value() };
        if (material_type == "emission") { // Also add to lights
          auto geometry_element = child_element->FirstChildElement("geometry");
          if (geometry_element) {
            std::string geometry_type{ geometry_element->FindAttribute("type")->Value() };
            if (geometry_type == "plane") {
              auto geometry = std::make_unique<Plane>(geometry_element);
              //LAND_INFO("Add plane light with area {}", geometry->GetArea());
              lights_.AddLight(
                std::move(geometry),
                material.emission,
                material.emission_strength);
            }
            else {
              throw "Unknown geometry type!";
            }
          }
          else {
            throw "Emission object has no element geometry!";
          }
        }

        // bunny, lucy contain transformtion attribute to move them to the correct place in scene
        glm::mat4 transformation = XmlComposeTransformMatrix(child_element);

        auto name_attribute = child_element->FindAttribute("name");
        if (name_attribute) {
          AddEntity(
            std::move(std::make_unique<AcceleratedMesh>(mesh)), material, transformation,
            std::string(name_attribute->Value()),
            speed);
          //LAND_INFO("Added entity {}", std::string(name_attribute->Value()));
        }
        else {
          AddEntity(
            std::move(std::make_unique<AcceleratedMesh>(mesh)), material, transformation, speed);
        }
      }
      else {
        LAND_ERROR("Unknown Element Type: {}", child_element->Value());
      }
    }
  }
  SetCameraToWorld(camera_to_world);
  UpdateEnvmapConfiguration();
}

std::vector<int> Scene::RayTracingPreSort_(const glm::vec3& origin, const glm::vec3& direction, float t_min, float t_max) const
{
  struct Ref {
    Ref(float dist, int idx): ref_dist{dist}, entity_index {idx} {}
    float ref_dist;
    int entity_index;
  };
  std::vector<Ref> list_to_sort;
  list_to_sort.reserve(entities_.size());
  //std::vector<bool> intersect_list(entities_.size());
  for (int i = 0; i < entities_.size(); i++) {
    auto& entity = entities_[i];
    auto model = entity.GetModel();
    // Transform direction according to entity property
    auto& transform = entities_[i].GetTransformMatrix();
    auto inv_transform = glm::inverse(transform);
    auto transformed_direction =
      glm::vec3{ inv_transform * glm::vec4{direction, 0.0f} };
    auto transformed_direction_length = glm::length(transformed_direction);
    if (transformed_direction_length < 1e-6) {
      continue;
    }
    auto acc_mesh = dynamic_cast<const AcceleratedMesh*>(model);
    if (acc_mesh == nullptr) {
      LAND_ERROR("Some entities did not use accelerated mesh!");
    }
    const AxisAlignedBoundingBox& box = acc_mesh->GetBoundingBox();
    glm::vec3 origin_trans = inv_transform * glm::vec4{ origin, 1.0f };
    //box.ShowBox();
    //LAND_INFO("Origin {}", glm::to_string(origin_trans));
    //LAND_INFO("Transformed direction {}, length {}", glm::to_string(transformed_direction / transformed_direction_length), transformed_direction_length);
    float range_min, range_max;
    bool intersect = box.IsIntersect(
      origin_trans,
      transformed_direction / transformed_direction_length, 
      t_min * transformed_direction_length, t_max * transformed_direction_length, &range_min, &range_max);
    if (intersect) {
      list_to_sort.emplace_back((range_min + range_max) / 2 / transformed_direction_length, i);
    }
    //intersect_list[i] = intersect;
  }
  auto compare = [this](const Ref& r1, const Ref& r2) -> bool {
    return r1.ref_dist < r2.ref_dist;
    };
  std::sort(list_to_sort.begin(), list_to_sort.end(), compare);
  std::vector<int> entities_order_list(list_to_sort.size());
  for (int i = 0; i < list_to_sort.size(); i++) {
    entities_order_list[i] = list_to_sort[i].entity_index;
  }
  //if (entities_order_list.size() == 1) {
  //  LAND_WARN("list size 1 with origin {}, direction {}", glm::to_string(origin), glm::to_string(direction));
  //}
  //if (entities_order_list.empty()) {
  //  LAND_ERROR("Empty list with origin {}, direction {}", glm::to_string(origin), glm::to_string(direction));
  //}
  //LAND_INFO("PreSort finishes with list size {}", entities_order_list.size());
  //show_vector(entities_order_list);
  return entities_order_list;
}

std::vector<int> Scene::RayTracingPreSort_(const glm::vec3& origin, const glm::vec3& direction, float time, float t_min, float t_max) const
{
  struct Ref {
    Ref(float dist, int idx) : ref_dist{ dist }, entity_index{ idx } {}
    float ref_dist;
    int entity_index;
  };
  std::vector<Ref> list_to_sort;
  list_to_sort.reserve(entities_.size());
  //std::vector<bool> intersect_list(entities_.size());
  for (int i = 0; i < entities_.size(); i++) {
    auto& entity = entities_[i];
    auto model = entity.GetModel();
    // Transform direction according to entity property
    auto& transform = entities_[i].GetTransformMatrix(time);
    //auto& transform = entities_[i].GetTransformMatrix();
    auto inv_transform = glm::inverse(transform);
    auto transformed_direction =
      glm::vec3{ inv_transform * glm::vec4{direction, 0.0f} };
    auto transformed_direction_length = glm::length(transformed_direction);
    if (transformed_direction_length < 1e-6) {
      continue;
    }
    auto acc_mesh = dynamic_cast<const AcceleratedMesh*>(model);
    if (acc_mesh == nullptr) {
      LAND_ERROR("Some entities did not use accelerated mesh!");
    }
    const AxisAlignedBoundingBox& box = acc_mesh->GetBoundingBox();
    glm::vec3 origin_trans = inv_transform * glm::vec4{ origin, 1.0f };
    //box.ShowBox();
    //LAND_INFO("Origin {}", glm::to_string(origin_trans));
    //LAND_INFO("Transformed direction {}, length {}", glm::to_string(transformed_direction / transformed_direction_length), transformed_direction_length);
    float range_min, range_max;
    bool intersect = box.IsIntersect(
      origin_trans,
      transformed_direction / transformed_direction_length,
      t_min * transformed_direction_length, t_max * transformed_direction_length, &range_min, &range_max);
    if (intersect) {
      list_to_sort.emplace_back((range_min + range_max) / 2 / transformed_direction_length, i);
    }
    //intersect_list[i] = intersect;
  }
  auto compare = [this](const Ref& r1, const Ref& r2) -> bool {
    return r1.ref_dist < r2.ref_dist;
    };
  std::sort(list_to_sort.begin(), list_to_sort.end(), compare);
  std::vector<int> entities_order_list(list_to_sort.size());
  for (int i = 0; i < list_to_sort.size(); i++) {
    entities_order_list[i] = list_to_sort[i].entity_index;
  }
  //if (entities_order_list.size() == 1) {
  //  LAND_WARN("list size 1 with origin {}, direction {}", glm::to_string(origin), glm::to_string(direction));
  //}
  //if (entities_order_list.empty()) {
  //  LAND_ERROR("Empty list with origin {}, direction {}", glm::to_string(origin), glm::to_string(direction));
  //}
  //LAND_INFO("PreSort finishes with list size {}", entities_order_list.size());
  //show_vector(entities_order_list);
  return entities_order_list;
}

}  // namespace sparks
