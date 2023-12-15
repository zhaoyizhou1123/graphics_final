#pragma once
#include "glm/glm.hpp"
#include <random>

namespace sparks {
class Camera {
 public:
  Camera(float fov = 60.0f, float aperture = 0.0f, float focal_length = 3.0f);
  [[nodiscard]] glm::mat4 GetProjectionMatrix(float aspect,
                                              float t_min,
                                              float t_max) const;

  /**
  * @brief Generate a ray acording to pixel, in camera world. 
  * The true origin and direction should be transformed to world space
  * @param range_low, range_high: The range on the image to sample. Both in [0,1]
  * @param rng: random number generator
  * @param origin: The resulting origin sampled on the lens (in camera space, will be converted to world space later)
  * @param direction: The direction from origin to object
  */
  void GenerateRay(float aspect,
                   glm::vec2 range_low,
                   glm::vec2 range_high,
                   glm::vec3 &origin,
                   glm::vec3 &direction,
                   std::mt19937& rng) const;
  bool ImGuiItems();
  void UpdateFov(float delta);
  [[nodiscard]] float GetFov() const {
    return fov_;
  }
  [[nodiscard]] float GetAperture() const {
    return aperture_;
  }
  [[nodiscard]] float GetFocalLength() const {
    return focal_length_;
  }
  [[nodiscard]] float GetClamp() const {
    return clamp_;
  }
  [[nodiscard]] float GetGamma() const {
    return gamma_;
  }

 private:
  float fov_{60.0f}; // in degree
  float aperture_{0.0f}; // radius of lens
  /* Not focus of camera. 
  * It is the distance between plane of focus (not focal plane) to lens(camera).
  * That is, rays emitting from plane of focus will converge to the camera sensor (image).
  */
  float focal_length_{3.0f};  
  float clamp_{100.0f};
  float gamma_{2.2f};
};
}  // namespace sparks
