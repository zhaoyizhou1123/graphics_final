#include "sparks/renderer/path_tracer.h"

#include "sparks/util/util.h"
#include "sparks/util/sample.h"
#include <glm/gtx/string_cast.hpp>

namespace sparks {
PathTracer::PathTracer(const RendererSettings *render_settings,
                       const Scene *scene,
                       unsigned int seed):
  rng_{seed}, bounce_cnt_{ 0 } {
  render_settings_ = render_settings;
  scene_ = scene;
}
PathTracer::PathTracer(const RendererSettings* render_settings, const Scene* scene):
  rng_{std::random_device()()}, bounce_cnt_ {0}
{
  render_settings_ = render_settings;
  scene_ = scene;
}

// No longer valid under current framework
glm::vec3 PathTracer::SampleRay(glm::vec3 origin,
                                glm::vec3 direction,
                                int x,
                                int y,
                                int sample) const {
  glm::vec3 throughput{1.0f};
  glm::vec3 radiance{0.0f};
  HitRecord hit_record;
  const int max_bounce = render_settings_->num_bounces;
  std::mt19937 rd(sample ^ x ^ y);
  float time = 0.0f;
  for (int i = 0; i < max_bounce; i++) {
    auto t = scene_->TraceRay(origin, direction, time, 1e-3f, 1e4f, &hit_record);
    if (t > 0.0f) {
      LAND_INFO("Get Intersection with distance {}", t);
      auto &material =
          scene_->GetEntity(hit_record.hit_entity_id).GetMaterial();
      if (material.material_type == MATERIAL_TYPE_EMISSION) {
        radiance += throughput * material.emission * material.emission_strength;
        break;
      } else {
        throughput *=
            material.albedo_color *
            glm::vec3{scene_->GetTextures()[material.albedo_texture_id].Sample(
                hit_record.tex_coord)};
        origin = hit_record.position;

        // The direction is fixed to the scene. No sampling. 
        // I guess this code assumes parallel light source, so the light direction is independent of origin.
        direction = scene_->GetEnvmapLightDirection(); 
        radiance += throughput * scene_->GetEnvmapMinorColor();
        throughput *=
            std::max(glm::dot(direction, hit_record.normal), 0.0f) * 2.0f;
        if (scene_->TraceRay(origin, direction, time, 1e-3f, 1e4f, nullptr) < 0.0f) {
          radiance += throughput * scene_->GetEnvmapMajorColor();
        }
        break;
      }
    } else {
      radiance += throughput * glm::vec3{scene_->SampleEnvmap(direction)};
      break;
    }
  }
  return radiance;
}

glm::vec3 PathTracer::SampleRayPathTrace(glm::vec3 origin,
                                         glm::vec3 direction,
                                         float time){
  //LAND_INFO("Path trace starts {}, {}", glm::to_string(origin), glm::to_string(direction));
  bounce_cnt_ = 0; // clear bounce_cnt_
  HitRecord hit_record;
  // Find intersection
  auto t = scene_->TraceRay(origin, direction, time, 1e-3f, 1e4f, &hit_record);
  //auto t = scene_->TraceRay(origin, direction, 1e-3f, 1e4f, &hit_record);
  //LAND_INFO("Finished ray tracing");
  if (t <= 0.0f) { // No intersection
    //return glm::vec3{scene_->SampleEnvmap(direction)}; // Check this
    return glm::vec3{ 0.0f };
  }
  // Has intersection
  auto shade_p = hit_record.position; // The position to shade
  glm::vec3 shade_dir = -direction; // The direction to shade

  // Test whether the intersection is light source
  auto& material =
      scene_->GetEntity(hit_record.hit_entity_id).GetMaterial(); // The hit object is stored in hit_record

  //auto normal = hit_record.normal;
  //if (material->material_type == MATERIAL_TYPE_EMISSION) { // light source, return directly
  //    return material->emission * material->emission_strength;
    
  // Not light source
  glm::vec3 color = Shade_(hit_record, shade_dir, time);
  color.x = clamp(color.x, 0.0f, render_settings_->max_color);
  color.y = clamp(color.y, 0.0f, render_settings_->max_color);
  color.z = clamp(color.z, 0.0f, render_settings_->max_color);
  return color;
  //return glm::vec3{};
}

glm::vec3 PathTracer::Shade_(const HitRecord& hit_record, const glm::vec3& dir_out, float time){
  const Material& material = scene_->GetEntity(hit_record.hit_entity_id).GetMaterial(); // material
  glm::vec3 p = hit_record.position; // position to shade
  //LAND_INFO("Bounce {}; position {}; in direction {}", bounce_cnt_, glm::to_string(p), glm::to_string(-dir_out) );
  // Get color according to texture
  //LAND_INFO("Texture coord {}", glm::to_string(hit_record.tex_coord));
  glm::vec3 hit_color = glm::vec3{ scene_->GetTextures()[material.albedo_texture_id].Sample(
                hit_record.tex_coord) } * material.albedo_color;
  glm::vec3 normal;
  //LAND_INFO("normal texture id {}", material.normal_texture_id);
  if (material.normal_texture_id == 1) { // default normal
    normal = hit_record.normal; // normal or geometry normal?
  }
  else {
    auto& tangent_space_normal = glm::vec3{ scene_->GetTextures()[material.normal_texture_id].Sample(
                hit_record.tex_coord) };
    //LAND_INFO("Normal texture id {}. Get tangent space normal {}", material.normal_texture_id, glm::to_string(tangent_space_normal));
    tangent_space_normal = tangent_space_normal * 2.0f - 1.0f; // convert to [-1,1]
    //LAND_INFO("Tangent space normal {}", glm::to_string(tangent_space_normal));
    // Compute TBN in model space
    if (glm::length(hit_record.tangent) < 1e-5) {
      LAND_ERROR("Hit record tangent is 0!");
    }
    auto& bitangent = glm::cross(hit_record.normal, hit_record.tangent);
    glm::mat3 TBN{ hit_record.tangent, bitangent, hit_record.normal };
    //LAND_INFO("Get TBN matrix {}", glm::to_string(TBN));
    normal = glm::normalize(TBN * tangent_space_normal);
    if (glm::dot(normal, hit_record.normal) < 0) { // Correct direction
      normal = -normal;
    }
   
  }
  if (glm::abs(glm::length(normal) - 1.0f) > 1e-5f) {
    LAND_ERROR("Unnormalized normal! {}", glm::to_string(normal));
  }
  if (material.material_type == MATERIAL_TYPE_EMISSION) { // emission
    //LAND_INFO("Material emission");
    // We may consider diffuse/specular light in principled BSDF
    //if (bounce_cnt_ >= 1) {
    //  LAND_INFO("Bounce to emission! {}", bounce_cnt_);
    //}
    glm::vec3 color = ShadeEmission_(
      dir_out,
      normal,
      material.emission,
      material.emission_strength);
    //LAND_INFO("Get emission light {}", glm::to_string(color));
    //if (glm::any(glm::lessThan(color, glm::vec3{ 0.0f }))) {
    //  LAND_INFO("Is front face {}", hit_record.front_face);
    //  LAND_INFO("Geometry normal {}", glm::to_string(hit_record.geometry_normal));
    //  LAND_INFO("Cosine {}", glm::dot(hit_record.normal, dir_out));
    //  LAND_ERROR("Negative color {} at postion {} with normal {}", glm::to_string(color), glm::to_string(p), glm::to_string(hit_record.normal) );
    //}
    //if (color.x != color.y) {
    //  LAND_WARN("Buggy color {}, emission {}", glm::to_string(color), glm::to_string(material.emission));
    //}
    return color;
  } 
  else if (material.material_type == MATERIAL_TYPE_LAMBERTIAN) { // diffuse
    //LAND_INFO("Material diffuse");
    return ShadeDiffuse_(
      p,
      dir_out,
      hit_color,
      normal,
      time
    );
  }
  else if (material.material_type == MATERIAL_TYPE_SPECULAR) { // specular
    //LAND_INFO("Material specular");
    //if (glm::abs(p.y - 330.0f) < 1e-3f) {
    //  LAND_INFO("Specular bounce at tall block top! {}", glm::to_string(p));
    //}
    return ShadeSpecular_(
      p,
      dir_out,
      normal,
      hit_color,
      time
    );
  }
  else if (material.material_type == MATERIAL_TYPE_TRANSMISSIVE) { // transmissive
    return ShadeTransmissive_(
      p,
      dir_out,
      normal,
      hit_color,
      material.ior,
      hit_record.front_face,
      time
    );
  }
  else if (material.material_type == MATERIAL_TYPE_PRINCIPLED) { // principled bsdf
    return ShadeBsdf_(p, dir_out, normal, material, hit_record.front_face, time);
  }
  else {
    LAND_ERROR("Unknown material type {}!", material.material_type);
    throw "Error occurs!";
  }
}

glm::vec3 PathTracer::ShadeDiffuse_(const glm::vec3& p, const glm::vec3& dir_out, const glm::vec3& albedo_color, const glm::vec3& normal, float time)
{
  if (glm::abs(glm::length(normal) - 1.0f) > 1e-3f) { // Check if normalized
    throw "Unnormalized normal!";
  }
  //if (glm::dot(dir_out, normal) < 0.0f) {
  //  LAND_ERROR("Normal direction incorrect! Normal {}, out ray {}", glm::to_string(normal), glm::to_string(dir_out));
  //}
  //LAND_INFO("Normal correct at {}", glm::to_string(p));
  // Compute direct light
  glm::vec3 radiance_dir{ 0.0f };
  // Sample light source, seed choice is unimportant
  int sample_light_idx;
  glm::vec3 sample_light_pos;
  float pdf_light = scene_->GetLights().Sample(&sample_light_idx, &sample_light_pos, rng_);
  const Light* sample_light = scene_->GetLights().GetLight(sample_light_idx);
  // Test blocking to sampled light source
  HitRecord hit_record;
  glm::vec3 ray = sample_light_pos - p; // ray to light
  //scene_->TraceRay(p, glm::normalize(ray), 1e-3f, 1e4f, &hit_record);
  scene_->TraceRay(p, glm::normalize(ray), time, 1e-3f, 1e4f, &hit_record);
  auto hit_normal = hit_record.normal; // normal of intersection
  if (glm::abs(glm::length(hit_normal) - 1.0f) > 1e-3f) { // Check if normalized
    throw "Unnormalized normal!";
  }
  if (glm::distance(hit_record.position, sample_light_pos) < 1e-3f && hit_record.front_face) { // No blocking
   //  TODO: Complete direct light
    //float cos_hit = glm::dot(normal, glm::normalize(ray));
    //float cos_light = glm::dot(hit_normal, glm::normalize(-ray));
    //if (cos_hit < 0.0f) {
    //  cos_hit = 0.0f;
    //}
    //if (cos_light < 0.0f) {
    //  cos_light = 0.0f;
    //}
    float cos_hit = glm::abs(glm::dot(normal, glm::normalize(ray)));
    float cos_light = glm::abs(glm::dot(hit_normal, glm::normalize(-ray)));
    //float cos_hit = 1.0f;
    //float cos_light = 1.0f;
    float square_dist = glm::dot(ray, ray); // squared distance from p to light
    radiance_dir = sample_light->emission * sample_light->emission_strength
      * albedo_color * sparks::INV_PI
      * cos_hit * cos_light / square_dist / pdf_light;
  }
  //LAND_INFO("Get direct light {}", glm::to_string(radiance_dir));

  // TODO: Complete indirect light
  glm::vec3 radiance_indir{ 0.0f };
  float sample_prob = std::uniform_real_distribution<float>(0.0f, 1.0f)(rng_);
  if (bounce_cnt_ < render_settings_->num_bounces 
      && sample_prob < render_settings_->prob_rr) 
  { // Bounce count did not reach limit and passed russian roulette
    float pdf;
    glm::vec3 ray_in_reverse = hemisphere_sample_cosine_weighted(normal, rng_, &pdf); // Sampled incident ray, but pointing outward
    //glm::vec3 ray_in_reverse = hemisphere_sample(normal, rng_);
    //pdf = 0.5 * INV_PI;
    HitRecord hit_record_ind;
    //float dist = scene_->TraceRay(p, ray_in_reverse, 1e-3f, 1e4f, &hit_record_ind);
    if (pdf > 0.0f) {
      float dist = scene_->TraceRay(p, ray_in_reverse, time, 1e-3f, 1e4f, &hit_record_ind);
      if (dist > 0.0f) { // hit
        const Material& hit_material = scene_->GetEntity(hit_record_ind.hit_entity_id).GetMaterial();
        if (hit_material.material_type != MATERIAL_TYPE_EMISSION) { // not emission
          bounce_cnt_++;
          //LAND_INFO("Diffuse bounce pos {}, dir {}, albedo {}", glm::to_string(p), glm::to_string(ray_in_reverse), glm::to_string(albedo_color));
          glm::vec3 color = Shade_(hit_record_ind, -ray_in_reverse, time);
          radiance_indir = color * (albedo_color * INV_PI)
            * glm::abs(glm::dot(normal, ray_in_reverse)) / pdf / render_settings_->prob_rr;
          //LAND_INFO("Get indirect light {}", glm::to_string(radiance_indir));
        }
      }
      else {
        //LAND_INFO("Diffuse bounce hit fail! pos {}, dir {}, albedo {}", glm::to_string(p), glm::to_string(ray_in_reverse), glm::to_string(albedo_color));
      }
    }
  }
  else {
    //LAND_INFO("Failed RR {}", sample_prob);
  }
  // Need ambient light?
  //glm::vec3 ambient_color = albedo_color * scene_->GetEnvmapMinorColor();
  glm::vec3 ambient_color{ 0.0f };
  //LAND_INFO("Direct light {}, indirect light {}, ambient light {}", 
  //  glm::to_string(radiance_dir), 
  //  glm::to_string(radiance_indir), 
  //  glm::to_string(ambient_color));
  glm::vec3 total_color = radiance_dir + radiance_indir + ambient_color;
  if (glm::any(glm::lessThan(total_color, glm::vec3{ 0.0f }))) {
    LAND_WARN("Negative color {}, dir {}, indir {}", glm::to_string(total_color), glm::to_string(radiance_dir), glm::to_string(radiance_indir));
  }
  return total_color;
}

glm::vec3 PathTracer::ShadeEmission_(const glm::vec3& dir_out, const glm::vec3& normal, const glm::vec3& emission, float emission_strength) const
{
  return emission * emission_strength * glm::dot(glm::normalize(dir_out), glm::normalize(normal));
}

glm::vec3 PathTracer::ShadeSpecular_(const glm::vec3& p, const glm::vec3& dir_out, const glm::vec3& normal, const glm::vec3& albedo_color, float time)
{
  if (glm::abs(glm::length(normal) - 1.0f) > 1e-3f) {
    throw "Unnormalized normal!";
  }
  if (glm::abs(glm::length(dir_out) - 1.0f) > 1e-3f) {
    throw "Unnormalized out ray!";
  }
  // If just reach limit, allow the ray to bounce back once, so it can get diffuse light or direct light
  if (bounce_cnt_ >= render_settings_->num_bounces + 1) {
    //LAND_WARN("Too many bounces {}", bounce_cnt_);
    //return albedo_color * scene_->GetEnvmapMinorColor();
    return glm::vec3{ 0.0f };
  }
  if (glm::dot(dir_out, normal) < 0.0f) {
    LAND_ERROR("Normal direction incorrect! Normal {}, out ray {}", glm::to_string(normal), glm::to_string(dir_out));
  }
  glm::vec3 dir_in_reverse = glm::reflect(-dir_out, normal); // the reverse direction of incident light
  HitRecord hit_record_ind;
  float dist = scene_->TraceRay(p, dir_in_reverse, time, 1e-3f, 1e4f, &hit_record_ind);
  if (dist > 0.0f) { // hit
    bounce_cnt_++;
    //LAND_INFO("Specular, p {}, in ray {}, out ray {}, next point {}",
    //  glm::to_string(p),
    //  glm::to_string(-dir_out),
    //  glm::to_string(dir_in_reverse),
    //  glm::to_string(hit_record_ind.position));
    return albedo_color * Shade_(hit_record_ind, -dir_in_reverse, time);
  }
  else {
    //return albedo_color * scene_->GetEnvmapMinorColor();
    return glm::vec3{ 0.0f };
  }
}

glm::vec3 PathTracer::ShadeTransmissive_(const glm::vec3& p, const glm::vec3& dir_out, const glm::vec3& normal, const glm::vec3& albedo_color, float ior, bool is_front, float time)
{
  if (glm::abs(glm::length(normal) - 1.0f) > 1e-3f) {
    throw "Unnormalized normal!";
  }
  if (glm::abs(glm::length(dir_out) - 1.0f) > 1e-3f) {
    throw "Unnormalized out ray!";
  }
  // If just reach limit, allow the ray to bounce back once, so it can get diffuse light or direct light
  if (bounce_cnt_ >= render_settings_->num_bounces + 1) {
    return glm::vec3{ 0.0f };
  }
  if (glm::dot(dir_out, normal) < 0.0f) {
    LAND_WARN("Bounce {}, normal direction incorrect! Normal {}, out ray {}, product {}", bounce_cnt_, glm::to_string(normal), glm::to_string(dir_out), glm::dot(dir_out, normal));
  }
  //LAND_INFO("Bounce {}, normal direction correct", bounce_cnt_);
  glm::vec3 dir_in = -dir_out; // reverse light path
  glm::vec3 dir_reflect = glm::reflect(dir_in, normal);

  float ior_in_over_refract = 1.0f / ior; // eta_i / eta_t
  if (!is_front) {
    ior_in_over_refract = ior;
  }
  glm::vec3 dir_refract = glm::refract(dir_in, normal, ior_in_over_refract);

  bool is_total_reflect = glm::length(dir_refract) < 1e-3f; // If total refract, dir_refract = (0,0,0)

  // Compute fresnel;
  float fr = 1.0f; // default: reflection only
  if (!is_total_reflect) { // Refraction exists, calculate true fr
    // Schlick¡¯s approximation
    float cos_thetai = -glm::dot(dir_in, normal);
    float r0 = (1 - ior) * (1 - ior) / (1 + ior) / (1 + ior); // fresnel term when thetai=0
    fr = r0 + (1 - r0) * (1 - cos_thetai) * (1 - cos_thetai) * (1 - cos_thetai) * (1 - cos_thetai) * (1 - cos_thetai);
  }

  // Sample to determine reflection/refraction
  float random_sample = std::uniform_real_distribution<float>(0.0f, 1.0f)(rng_);
  if (is_total_reflect || random_sample < fr) { // Reflect
    HitRecord hit_record_ind;
    float dist = scene_->TraceRay(p, dir_reflect, time, 1e-3f, 1e4f, &hit_record_ind);
    if (dist > 0.0f) { // hit
      bounce_cnt_++;
      return albedo_color * Shade_(hit_record_ind, -dir_reflect, time);
    }
    else {
      return glm::vec3{ 0.0f };
    }
  }
  else { // Refract
    // Compute refract light
    glm::vec3 color_refract{ 0.0f };
    HitRecord hit_record_refract;
    float dist_refract = scene_->TraceRay(p, dir_refract, time, 1e-3f, 1e4f, &hit_record_refract);
    if (dist_refract > 0.0f) { // hit
      bounce_cnt_++;
      return albedo_color * Shade_(hit_record_refract, -dir_refract, time);
    }
    else {
      return glm::vec3{ 0.0f };
    }
  }
}

// Based on https://github.com/mmp/pbrt-v3/
glm::vec3 PathTracer::ShadeBsdf_(
  const glm::vec3& p,
  const glm::vec3& dir_out,
  const glm::vec3& normal, 
  const Material& material, 
  bool is_front_face,
  float time)
{
  glm::vec3 color = material.albedo_color;
  float metallic_weight = material.metallic;
  float ior = material.ior;
  float spec_trans = material.spec_trans;
  float diffuse_weight = (1 - metallic_weight) * (1 - spec_trans);
  float diff_trans = material.diff_trans; // 0: all reflect -> 1: all transmit
  float rough = material.roughness;
  std::vector<std::unique_ptr<Bxdf> > bxdfs; // store a list of bxdfs
  std::vector<float> bxdf_weights; // store sample weights
  
  if (diffuse_weight > 0) {
    if (material.thin) {
      float flat = material.flatness;
      // TODO: Push diffuse and fakess into bxdfs
      bxdfs.push_back(std::make_unique<DisneyDiffuse>(color));
      bxdf_weights.push_back(diffuse_weight * (1 - flat) * (1 - diff_trans));
      bxdfs.push_back(std::make_unique<DisneyFakess>(color, rough));
      bxdf_weights.push_back(diffuse_weight * flat * (1 - diff_trans));
    }
    else {
      LAND_ERROR("Not implemented for thick material!");
    }
    // Retro-reflection.
    bxdfs.push_back(std::make_unique<DisneyRetro>(color, rough));
    bxdf_weights.push_back(diffuse_weight);
  }
  // microfacet distribution
  float aspect = 1.0f; // fix anisotrophic=0.0f
  float alpha_x = glm::max(1e-3f, rough * rough / aspect);
  float alpha_y = glm::max(1e-3f, rough * rough * aspect);
  auto distrib = std::make_unique<MicrofacetDistribution>(alpha_x, alpha_y, normal);

  glm::vec3 c_spec0 = linear_interpolate(metallic_weight, glm::vec3{ (ior - 1) * (ior - 1) / (ior + 1) / (ior + 1) }, color);
  auto fresnel = std::make_unique<Fresnel>(c_spec0, metallic_weight, ior);
  // Add weight according to spec_trans
  float reflect_weight = glm::max(1e-3f, 1 - spec_trans);
  bxdfs.push_back(std::make_unique<MicrofacetReflection>(glm::vec3{ 1.0f / reflect_weight }, std::move(distrib), std::move(fresnel)));
  bxdf_weights.push_back(reflect_weight);

  // transmission
  if (spec_trans > 0.0f) {
    // Walter et al's model, with the provided transmissive term scaled
    // by sqrt(color), so that after two refractions, we're back to the
    // provided color.
    glm::vec3 T = glm::sqrt(color);
    if (material.thin) {
      // Scale roughness based on IOR (Burley 2015, Figure 15).
      float rscaled = (0.65f * ior - 0.35f) * rough;
      float alpha_x = glm::max(1e-3f, rscaled * rscaled / aspect);
      float alpha_y = glm::max(1e-3f, rscaled * rscaled * aspect);
      auto scaled_distrib = std::make_unique<MicrofacetDistribution>(alpha_x, alpha_y, normal);
      bxdfs.push_back(std::make_unique<MicrofacetTransmission>(T, std::move(scaled_distrib), 1.0f, ior));
      bxdf_weights.push_back(spec_trans);
    }
    else {
      LAND_ERROR("Not implemented!");
    }
  }
  if (material.thin) {
    bxdfs.push_back(std::make_unique<LambertianTransmission>(color));
    bxdf_weights.push_back(diff_trans);
  }

  // Sample bxdf
  //int bxdf_idx = std::uniform_int_distribution<int>(0, bxdfs.size() - 1)(rng_);
  int bxdf_idx = std::discrete_distribution<int>(bxdf_weights.begin(), bxdf_weights.end())(rng_);
  Bxdf* bxdf = bxdfs[bxdf_idx].get();

  // Similar to diffuse
  if (glm::abs(glm::length(normal) - 1.0f) > 1e-3f) { // Check if normalized
    throw "Unnormalized normal!";
  }
  // Direct light
  glm::vec3 radiance_dir{ 0.0f };
  int sample_light_idx;
  glm::vec3 sample_light_pos;
  // IS method 1: Sampling the light
  float pdf_light = scene_->GetLights().Sample(&sample_light_idx, &sample_light_pos, rng_);
  const Light* sample_light = scene_->GetLights().GetLight(sample_light_idx);
  // Test blocking to sampled light source
  HitRecord hit_record_dir;
  glm::vec3 ray = sample_light_pos - p; // ray to light
  //scene_->TraceRay(p, glm::normalize(ray), 1e-3f, 1e4f, &hit_record);
  scene_->TraceRay(p, glm::normalize(ray), time, 1e-3f, 1e4f, &hit_record_dir);
  auto hit_normal = hit_record_dir.normal; // normal of intersection
  if (glm::abs(glm::length(hit_normal) - 1.0f) > 1e-3f) { // Check if normalized
    throw "Unnormalized normal!";
  }
  if (glm::distance(hit_record_dir.position, sample_light_pos) < 1e-3f) { // No blocking
    float cos_hit = glm::dot(normal, glm::normalize(ray));
    float cos_light = glm::dot(hit_normal, glm::normalize(-ray));
    // In case cosine < 0 
    if (cos_hit < 0.0f) {
      cos_hit = 0.0f;
    }
    if (cos_light < 0.0f) {
      cos_light = 0.0f;
    }
    // Compute MIS weight
    float pdf_bsdf = bxdf->GetPdf(normal, dir_out, glm::normalize(-ray), is_front_face);
    float weight = power_heuristic(1, pdf_light, 1, pdf_bsdf);
    float square_dist = glm::dot(ray, ray); // squared distance from p to light
    radiance_dir += sample_light->emission * sample_light->emission_strength
      * bxdf->GetBsdf(normal, dir_out, glm::normalize(-ray), is_front_face)
      * cos_hit * cos_light / square_dist / pdf_light * weight;
    //if (glm::any(glm::lessThan(radiance_dir, glm::vec3{ 0.0f }))) {
    //  LAND_ERROR("Negative color, bsdf {}, cos_hit {}, cos_light {}",
    //    glm::to_string(bxdf->GetBsdf(normal, dir_out, glm::normalize(-ray))),
    //    cos_hit, cos_light);
    //}
  }
  // IS method 2: Sampling the bsdf
  float pdf_dir_bsdf;
  //glm::vec3 ray_in_reverse = hemisphere_sample_cosine_weighted(normal, rng_, &pdf_indir); // Sampled incident ray, but pointing outward
  glm::vec3 ray_in;
  glm::vec3 bsdf = bxdf->SampleRayIn(normal, dir_out, &ray_in, &pdf_dir_bsdf, rng_, is_front_face);
  if (pdf_dir_bsdf > 0.0f) { // Only do the following if the sampled ray is valid
    glm::vec3 ray_in_reverse = -ray_in;
    //glm::vec3 ray_in_reverse = hemisphere_sample(normal, rng_);
    //pdf = 0.5 * INV_PI;
    HitRecord hit_record_dir_bsdf;
    //float dist = scene_->TraceRay(p, ray_in_reverse, 1e-3f, 1e4f, &hit_record_ind);
    float dist = scene_->TraceRay(p, ray_in_reverse, time, 1e-3f, 1e4f, &hit_record_dir_bsdf);
    if (dist > 0.0f) { // hit
      const Material& hit_material = scene_->GetEntity(hit_record_dir_bsdf.hit_entity_id).GetMaterial();
      if (hit_material.material_type == MATERIAL_TYPE_EMISSION) { // is a light source
        glm::vec3 color = Shade_(hit_record_dir_bsdf, -ray_in_reverse, time);
        // Compute MIS weight
        float pdf_light = 1.0f / scene_->GetLights().GetTotalArea();
        float weight = power_heuristic(1, pdf_dir_bsdf, 1, pdf_light);
        radiance_dir += color * bsdf
          * glm::abs(glm::dot(normal, ray_in_reverse)) / pdf_dir_bsdf * weight;
      }
      //if (glm::any(glm::lessThan(radiance_indir, glm::vec3{ 0.0f }))) {
      //  LAND_ERROR("Negative color {}, bsdf {}, cosine {}", glm::to_string(radiance_indir), glm::to_string(bsdf), glm::dot(normal, ray_in_reverse));
      //}
    }
  }
  // Indirect light
  glm::vec3 radiance_indir{ 0.0f };
  float sample_prob = std::uniform_real_distribution<float>(0.0f, 1.0f)(rng_);
  if (bounce_cnt_ < render_settings_->num_bounces
    && sample_prob < render_settings_->prob_rr)
  { // Bounce count did not reach limit and passed russian roulette
    float pdf_indir;
    //glm::vec3 ray_in_reverse = hemisphere_sample_cosine_weighted(normal, rng_, &pdf_indir); // Sampled incident ray, but pointing outward
    glm::vec3 ray_in;
    glm::vec3 bsdf = bxdf->SampleRayIn(normal, dir_out, &ray_in, &pdf_indir, rng_, is_front_face);
    if (pdf_indir > 0.0f) { // Only do the following if the sampled ray is valid
      glm::vec3 ray_in_reverse = -ray_in;
      //glm::vec3 ray_in_reverse = hemisphere_sample(normal, rng_);
      //pdf = 0.5 * INV_PI;
      HitRecord hit_record_ind;
      //float dist = scene_->TraceRay(p, ray_in_reverse, 1e-3f, 1e4f, &hit_record_ind);
      float dist = scene_->TraceRay(p, ray_in_reverse, time, 1e-3f, 1e4f, &hit_record_ind);
      if (dist > 0.0f) { // hit
        const Material& hit_material = scene_->GetEntity(hit_record_ind.hit_entity_id).GetMaterial();
        if (hit_material.material_type != MATERIAL_TYPE_EMISSION) { // not emission
          bounce_cnt_++;
          glm::vec3 color = Shade_(hit_record_ind, -ray_in_reverse, time);
          radiance_indir = color * bsdf
            * glm::abs(glm::dot(normal, ray_in_reverse)) / pdf_indir / render_settings_->prob_rr;
        }
        if (glm::any(glm::lessThan(radiance_indir, glm::vec3{ 0.0f }))) {
          LAND_ERROR("Negative color {}, bsdf {}, cosine {}", glm::to_string(radiance_indir), glm::to_string(bsdf), glm::dot(normal, ray_in_reverse));
        }
      }
    }
  }
  return radiance_dir + radiance_indir;
}
}  // namespace sparks
