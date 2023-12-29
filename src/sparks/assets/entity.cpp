#include "sparks/assets/entity.h"
#include "glm/gtc/matrix_transform.hpp"

namespace sparks {

const Model *Entity::GetModel() const {
  return model_.get();
}

glm::mat4 &Entity::GetTransformMatrix() {
  return transform_;
}

const glm::mat4 &Entity::GetTransformMatrix() const {
  return transform_;
}

// Return a reference of temp variable will cause bug!
glm::mat4 Entity::GetTransformMatrix(float time) const
{
  glm::vec3 displace = time * speed_; // displacement
  glm::mat4 translation = glm::translate(glm::mat4{ 1.0f }, displace);
  return translation * transform_;
}

Material &Entity::GetMaterial() {
  return material_;
}

const Material &Entity::GetMaterial() const {
return material_;
}

const std::string &Entity::GetName() const {
  return name_;
}

const glm::vec3& Entity::GetSpeed() const
{
  return speed_;
}

}  // namespace sparks
