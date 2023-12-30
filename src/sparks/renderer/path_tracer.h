#pragma once
#include "random"
#include "sparks/assets/scene.h"
#include "sparks/renderer/renderer_settings.h"
#include "sparks/materials/bxdfs_all.h"
#include "sparks/util/distribution.h"

namespace sparks {
class PathTracer {
 public:
  PathTracer(const RendererSettings *render_settings, const Scene *scene, unsigned int seed);
  PathTracer(const RendererSettings* render_settings, const Scene* scene);

  /**
  @brief Get the color sampled from one ray
  @param origin, camera position
  @param direction, ray direction
  @param x&y, pixel position
  @param sample, random seed?
  x,y,sample only serve to generate seed.
  @return color
  */
  [[nodiscard]] glm::vec3 SampleRay(glm::vec3 origin,
                                    glm::vec3 direction,
                                    int x,
                                    int y,
                                    int sample) const;

  /**
  @brief Get the color sampled from one ray.
  Path tracing ray sampling method. It will not be called recursively
  @param origin, camera position
  @param direction, ray direction
  @param bounce_cnt, counter of recursion, avoid infinite recursion (like two mirrors)
  @return color
  */
  [[nodiscard]] glm::vec3 SampleRayPathTrace(
    glm::vec3 origin,
    glm::vec3 direction,
    float time);

  void SetSeed(int seed) {
    rng_.seed(seed);
  }

 private:
  const RendererSettings *render_settings_{};
  const Scene *scene_{};
  std::mt19937 rng_{ std::random_device()() };
  int bounce_cnt_{ 0 }; // number of bounces

  /* @brief Get the color of scene point p given out ray direction, using path tracing.
  * Will be called recursively
  * @param p, the point on scene. p is guaranteed not to be on light source
  * @param dir_out, the out direction
  * @param seed: Only used as seed for sampling.
  */
  [[nodiscard]] glm::vec3 Shade_(const HitRecord& hit_record, const glm::vec3& dir_out, float time);
  /* @brief Compute light for diffuse material
  * @param p, position on diffuse material 
  * @param dir_out, normalized and points outward, we collect color observed from that direction
  * @param albedo_color, the base color
  * @param normal, the normal at p
  */
  [[nodiscard]] glm::vec3 ShadeDiffuse_(const glm::vec3& p, const glm::vec3& dir_out, const glm::vec3& albedo_color, const glm::vec3& normal, float time);
  [[nodiscard]] glm::vec3 ShadeSpecular_(
    const glm::vec3& p,
    const glm::vec3& dir_out,
    const glm::vec3& normal,
    const glm::vec3& albedo_color,
    float time);
  [[nodiscard]] glm::vec3 ShadeTransmissive_(
    const glm::vec3& p,
    const glm::vec3& dir_out,
    const glm::vec3& normal,
    const glm::vec3& albedo_color,
    float ior, 
    bool is_front,
    float time);
  [[nodiscard]] glm::vec3 ShadeBsdf_(
    const glm::vec3& p,
    const glm::vec3& dir_out, 
    const glm::vec3& normal,
    const Material& material, 
    bool is_front_face,
    float time);
  // Only consider emission light
  [[nodiscard]] glm::vec3 ShadeEmission_( 
    const glm::vec3& dir_out, 
    const glm::vec3& normal,
    const glm::vec3& emission,
    float emission_strength) const;

};
}  // namespace sparks
