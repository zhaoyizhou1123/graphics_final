#pragma once
#include "memory"
#include "sparks/assets/material.h"
#include "sparks/assets/mesh.h"
#include "sparks/assets/model.h"

namespace sparks {
/* @brief The objects in the scene.
* Stores model, material, etc.
*/
class Entity {
 public:
  template <class ModelType>
  Entity(const ModelType &model,
         const Material &material,
         const glm::mat4 &transform = glm::mat4{1.0f}) {
    model_ = std::make_unique<ModelType>(model);
    material_ = material;
    transform_ = transform;
    name_ = model_->GetDefaultEntityName();
  }

  template <class ModelType>
  Entity(std::unique_ptr<ModelType> model,
    const Material& material,
    const glm::mat4& transform = glm::mat4{ 1.0f }) {
    model_ = std::move(model);
    material_ = material;
    transform_ = transform;
    name_ = model_->GetDefaultEntityName();
  }

  template <class ModelType>
  Entity(const ModelType &model,
         const Material &material,
         const glm::mat4 &transform,
         const std::string &name) {
    model_ = std::make_unique<ModelType>(model);
    material_ = material;
    transform_ = transform;
    name_ = name;
  }

  template <class ModelType>
  Entity(std::unique_ptr<ModelType> model,
    const Material& material,
    const glm::mat4& transform,
    const std::string& name) {
    model_ = std::move(model);
    material_ = material;
    transform_ = transform;
    name_ = name;
  }

  [[nodiscard]] const Model *GetModel() const;
  [[nodiscard]] glm::mat4 &GetTransformMatrix();
  [[nodiscard]] const glm::mat4 &GetTransformMatrix() const;
  [[nodiscard]] Material &GetMaterial();
  [[nodiscard]] const Material &GetMaterial() const;
  [[nodiscard]] const std::string &GetName() const;

 private:
  std::unique_ptr<Model> model_;
  Material material_{};
  glm::mat4 transform_{1.0f};
  std::string name_;
};
}  // namespace sparks
