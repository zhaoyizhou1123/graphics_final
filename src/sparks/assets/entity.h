#pragma once
#include "memory"
#include "sparks/assets/material.h"
#include "sparks/assets/mesh.h"
#include "sparks/assets/model.h"
#include "glm/gtx/string_cast.hpp"

namespace sparks {
/* @brief The objects in the scene.
* Stores model, material, etc.
*/
class Entity {
 public:
  template <class ModelType>
  Entity(const ModelType &model,
         const Material &material,
         const glm::mat4 &transform = glm::mat4{1.0f},
         const glm::vec3 &speed = glm::vec3{0.0f}) {
    model_ = std::make_unique<ModelType>(model);
    material_ = material;
    transform_ = transform;
    name_ = model_->GetDefaultEntityName(),
    speed_ = speed;
  }

  template <class ModelType>
  Entity(std::unique_ptr<ModelType> model,
    const Material& material,
    const glm::mat4& transform = glm::mat4{ 1.0f },
    const glm::vec3& speed = glm::vec3{ 0.0f }) {
    model_ = std::move(model);
    material_ = material;
    transform_ = transform;
    name_ = model_->GetDefaultEntityName();
    speed_ = speed;
  }

  template <class ModelType>
  Entity(const ModelType &model,
         const Material &material,
         const glm::mat4 &transform,
         const std::string &name,
         const glm::vec3& speed = glm::vec3{ 0.0f }) {
    model_ = std::make_unique<ModelType>(model);
    material_ = material;
    transform_ = transform;
    name_ = name;
    speed_ = speed;
  }

  template <class ModelType>
  Entity(std::unique_ptr<ModelType> model,
    const Material& material,
    const glm::mat4& transform,
    const std::string& name,
    const glm::vec3& speed = glm::vec3{ 0.0f }) {
    model_ = std::move(model);
    material_ = material;
    transform_ = transform;
    name_ = name;
    speed_ = speed;
  }

  [[nodiscard]] const Model *GetModel() const;
  [[nodiscard]] glm::mat4 &GetTransformMatrix();
  [[nodiscard]] const glm::mat4 &GetTransformMatrix() const;
  // Consider motion blur
  [[nodiscard]] glm::mat4 GetTransformMatrix(float time) const;

  [[nodiscard]] Material &GetMaterial();
  [[nodiscard]] const Material &GetMaterial() const;
  [[nodiscard]] const std::string &GetName() const;
  [[nodiscard]] const glm::vec3& GetSpeed() const;

 private:
  std::unique_ptr<Model> model_;
  Material material_{};
  glm::mat4 transform_{1.0f};
  std::string name_;
  glm::vec3 speed_{ 0.0f }; // moving speed in world space
};
}  // namespace sparks
