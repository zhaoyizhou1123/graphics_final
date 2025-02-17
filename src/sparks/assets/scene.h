#pragma once
#include "memory"
#include "sparks/assets/camera.h"
#include "sparks/assets/entity.h"
#include "sparks/assets/material.h"
#include "sparks/assets/mesh.h"
#include "sparks/assets/texture.h"
#include "sparks/assets/util.h"
#include "sparks/assets/light.h"
#include "vector"

namespace sparks {
class Scene {
 public:
  Scene();
  explicit Scene(const std::string &filename);
  int AddTexture(const Texture &texture,
                 const std::string &name = "Unnamed Texture");
  [[nodiscard]] const std::vector<Texture> &GetTextures() const;
  [[nodiscard]] const Texture &GetTexture(int texture_id) const;
  [[nodiscard]] int GetTextureCount() const;
  [[nodiscard]] std::vector<const char *> GetTextureNameList() const;

  template <class... Args>
  int AddEntity(Args... args) {
    entities_.emplace_back(args...);
    return int(entities_.size() - 1);
  }
  template <class T, class... Args>
  int AddEntity(std::unique_ptr<T> ptr, Args... args) {
    entities_.emplace_back(std::move(ptr), args...);
    return int(entities_.size() - 1);
  }

  [[nodiscard]] Entity &GetEntity(int entity_index);
  [[nodiscard]] const Entity &GetEntity(int entity_index) const;
  [[nodiscard]] std::vector<Entity> &GetEntities();
  [[nodiscard]] const std::vector<Entity> &GetEntities() const;
  [[nodiscard]] int GetEntityCount() const;
  [[nodiscard]] std::vector<const char *> GetEntityNameList() const;

  [[nodiscard]] Lights& GetLights();
  [[nodiscard]] const Lights& GetLights() const;

  void SetCamera(const Camera &camera);
  Camera &GetCamera();
  [[nodiscard]] const Camera &GetCamera() const;
  [[nodiscard]] glm::mat4 GetCameraToWorld() const;
  void SetCameraToWorld(const glm::mat4 &camera_to_world);

  int &GetEnvmapId();
  [[nodiscard]] const int &GetEnvmapId() const;
  float &GetEnvmapOffset();
  [[nodiscard]] const float &GetEnvmapOffset() const;
  glm::vec3 &GetCameraPosition();
  [[nodiscard]] const glm::vec3 &GetCameraPosition() const;
  float &GetCameraSpeed();
  [[nodiscard]] const float &GetCameraSpeed() const;
  glm::vec3 &GetCameraPitchYawRoll();
  [[nodiscard]] const glm::vec3 &GetCameraPitchYawRoll() const;

  void Clear();

  // Update envmap_light_direction_, envmap_minor_color_, envmap_major_color_
  void UpdateEnvmapConfiguration();

  [[nodiscard]] glm::vec3 GetEnvmapLightDirection() const;
  [[nodiscard]] const glm::vec3 &GetEnvmapMinorColor() const;
  [[nodiscard]] const glm::vec3 &GetEnvmapMajorColor() const;
  [[nodiscard]] const std::vector<float> &GetEnvmapCdf() const;
  ;
  [[nodiscard]] glm::vec4 SampleEnvmap(const glm::vec3 &direction) const;

  /*@return t: The distance of intersection ?
  * @param direction: Should be normalized.
  * @param time: for motion blur
  */ 

  float TraceRay(const glm::vec3 &origin,
                 const glm::vec3 &direction,
                 float time,
                 float t_min,
                 float t_max,
                 HitRecord *hit_record) const;
  float TraceRay(const glm::vec3& origin,
    const glm::vec3& direction,
    float t_min,
    float t_max,
    HitRecord* hit_record) const;

  /* @brief Sample a ray for path tracing.
  * @param pos, p
  * @param ray_out, wo
  * @param material, the material of p
  * @return 
  *   ray_in, sampled wi
  *   weight, for diffuse, it's fr*cos/pdf_hemi/p_rr; for specular, it's 1; for principled, it could be other
  */
  void SampleRay(
    const glm::vec3& pos,
    const glm::vec3& ray_out,
    const Material& material,
    glm::vec3& ray_in,
    float& weight
  );

  bool TextureCombo(const char *label, int *current_item) const;
  bool EntityCombo(const char *label, int *current_item) const;
  int LoadTexture(const std::string &file_path);
  int LoadObjMesh(const std::string &file_path);

 private:
  std::vector<Texture> textures_;
  std::vector<std::string> texture_names_;

  std::vector<Entity> entities_; // The objects in the scene ?
  Lights lights_; // Light sources

  int envmap_id_{1}; // texture id
  float envmap_offset_{0.0f}; // Fixed, no method to change it?
  std::vector<float> envmap_cdf_;
  glm::vec3 envmap_light_direction_{0.0f, 1.0f, 0.0f};
  glm::vec3 envmap_major_color_{0.5f};
  glm::vec3 envmap_minor_color_{0.3f};

  glm::vec3 camera_position_{0.0f};
  float camera_speed_{3.0f};
  glm::vec3 camera_pitch_yaw_roll_{0.0f, 0.0f, 0.0f};
  Camera camera_{};

  /*@brief Preprocess the order of entities before ray tracing
  * @return a list of entity indices that need to test, and indicates the order to test
  */
  std::vector<int> RayTracingPreSort_(const glm::vec3& origin,
                                      const glm::vec3& direction,
                                      float time,
                                      float t_min,
                                      float t_max) const;
  std::vector<int> RayTracingPreSort_(const glm::vec3& origin,
    const glm::vec3& direction,
    float t_min,
    float t_max) const;
};
}  // namespace sparks
