
struct Material {
  int albedo_texture_id;
  int normal_texture_id;
  uint material_type;
  vec3 albedo_color;
  float metallic;
  float ior;
  float roughness;
  float spec_trans;
  bool thin;
  float flatness;
  float diff_trans;
  vec3 emission;
  float emission_strength;
  float alpha;
};

#define MATERIAL_TYPE_LAMBERTIAN 0
#define MATERIAL_TYPE_SPECULAR 1
#define MATERIAL_TYPE_TRANSMISSIVE 2
#define MATERIAL_TYPE_PRINCIPLED 3
#define MATERIAL_TYPE_EMISSION 4
