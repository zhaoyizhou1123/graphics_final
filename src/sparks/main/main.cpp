#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "glm/gtc/matrix_transform.hpp"
#include "iostream"
#include "sparks/sparks.h"
#include "tiny_obj_loader.h"
#include <exception>

// Testing include
#include "sparks/util/sample.h"
#include <random>
#include "glm/gtx/string_cast.hpp"
#include "glm/ext.hpp"
#include "sparks/assets/light.h"
#include "sparks/geometries/plane.h"
#include "sparks/assets/accelerated_mesh.h"
#include "sparks/assets/scene.h"
#include "sparks/acceleration/tree.hpp"

ABSL_FLAG(bool,
          validation_layer,
          grassland::vulkan::kDefaultEnableValidationLayers,
          "Enable Vulkan validation layer");
ABSL_FLAG(uint32_t, width, 1920, "Window width");
ABSL_FLAG(uint32_t, height, 1080, "Window height");
ABSL_FLAG(bool, vkrt, false, "Use Vulkan Ray Tracing pipeline");
ABSL_FLAG(int, device, -1, "Select physical device manually");

ABSL_FLAG(bool, test, false, "True if testing");

void RunApp(sparks::Renderer *renderer);

void test_main(); 

int main(int argc, char *argv[]) {
    try {
      LAND_INFO("width {}, height {}, vkrt {}, test {}", 
        absl::GetFlag(FLAGS_width),
        absl::GetFlag(FLAGS_height),
        absl::GetFlag(FLAGS_vkrt),
        absl::GetFlag(FLAGS_test));
      bool is_test = absl::GetFlag(FLAGS_test);
      if (!is_test) {
        absl::SetProgramUsageMessage("Usage");
        absl::ParseCommandLine(argc, argv);
        sparks::RendererSettings renderer_settings; // Default renderer setting
        sparks::Renderer renderer(renderer_settings);
        RunApp(&renderer);
      }
      else {
        test_main();
      }
    }
    catch (const std::exception & msg) {
        std::cerr << msg.what() << std::endl;
    }
}

void RunApp(sparks::Renderer *renderer) {
#if defined(_WIN32)
  SetConsoleOutputCP(65001);
#endif
  sparks::AppSettings app_settings;
  app_settings.validation_layer = absl::GetFlag(FLAGS_validation_layer);
  app_settings.width = absl::GetFlag(FLAGS_width);
  app_settings.height = absl::GetFlag(FLAGS_height);
  app_settings.hardware_renderer = absl::GetFlag(FLAGS_vkrt);
  app_settings.selected_device = absl::GetFlag(FLAGS_device);
  sparks::App app(renderer, app_settings);
  app.Run();
}

void show_box(const sparks::AxisAlignedBoundingBox& box) {
  std::cout << "Box: (" << box.x_low << ", " << box.x_high << ") x ";
  std::cout << "(" << box.y_low << ", " << box.y_high << ") x ";
  std::cout << "(" << box.z_low << ", " << box.z_high << ")\n";
}

void show_record(const sparks::HitRecord& hit_record) {
  std::cout << "Position: " << glm::to_string(hit_record.position) << std::endl;
  std::cout << "Normal: " << glm::to_string(hit_record.normal) << std::endl;
}

void traverse(sparks::AcceleratedMesh::BvhNode* node, int& total_cnt) {
  if (node->content->is_leaf) {
    //auto f_indices = node->content->face_indices;
    //for (auto itr = f_indices.begin(); itr != f_indices.end(); itr++) {
    //  std::cout << *itr << " ";
    //}
    //std::cout << std::endl;
    //std::cout << "Level " << cnt << ", size " << node->content->face_indices.size() << std::endl;
    show_box(node->content->box);
    total_cnt += node->content->face_indices.size();
    return;
  }
  else {
    traverse(node->left_child.get(), total_cnt);
    traverse(node->right_child.get(), total_cnt);
  }
}

void test_main() {
  //sparks::Mesh mesh;
  //std::string file_path("../../meshes/bunny.obj");
  //bool success = sparks::Mesh::LoadObjFile(file_path, mesh);
  //if (!success) {
  //  LAND_ERROR("Load file failed!");
  //}
  //sparks::AcceleratedMesh acc_mesh(mesh);
  //auto bvh = acc_mesh.GetBvh();
  //if (bvh == nullptr) {
  //  LAND_ERROR("Null bvh!");
  //}
  //auto root = bvh->GetRoot();
  //if (root == nullptr) {
  //  LAND_ERROR("Null root");
  //}
  //int cnt = 0;
  //traverse(root, cnt);

  //LAND_INFO("Bvh has stored {} faces", cnt);
  // 
  sparks::Scene scene("../../scenes/cornell_specular.xml");

  glm::vec3 origin{ 250.0f, 250.0f, -80.0f };
  glm::vec3 direction = glm::normalize(glm::vec3{ 0.2f, 1.0f, 1.0f });
  //sparks::HitRecord hit_record;
  //float t = scene.TraceRay(origin, direction, 1e-3, 1e4, &hit_record);
  //LAND_INFO("t: {}", t);
  sparks::RendererSettings renderer_setting;
  sparks::PathTracer path_tracer(&renderer_setting, &scene);
  std::string seed;
  while (true) {
    std::cout << "Input a seed, enter 's' to stop: ";
    std::cin >> seed;
    if (seed == "s") {
      break;
    }
    path_tracer.SetSeed(std::stoi(seed));
    glm::vec3 color = path_tracer.SampleRayPathTrace(origin, direction);
    LAND_INFO("Seed {}, origin {}, direction {}, color {}", seed, glm::to_string(origin), glm::to_string(direction), glm::to_string(color));
  }

}
