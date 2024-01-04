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
  //std::string file_path("../../meshes/sphere.obj");
  //bool success = sparks::Mesh::LoadObjFile(file_path, mesh);
  //if (!success) {
  //  LAND_ERROR("Load file failed!");
  //}
  //sparks::AcceleratedMesh acc_mesh(mesh);
  //acc_mesh.Check();
  //
  //glm::vec3 origin{ -5.0f, -2.412321f, -3.841506f };
  //glm::vec3 direction = glm::normalize(glm::vec3{ 0.687533, 0.392439, 0.610975 });

  //sparks::HitRecord hit_record;
  //float t = acc_mesh.TraceRayImprove(origin, direction, 1e-3, -1.0f, &hit_record);
  //LAND_INFO("Distance {}", t);

  //LAND_INFO("Bvh has stored {} faces", cnt);
  // 
  //sparks::Scene scene("../../scenes/cornell_v2.xml");

  //glm::vec3 origin{ 278.0f, 273.0f, -800.0f };
  //glm::vec3 dest{ 350.0f, 548.8f, 350.0f };
  ///*glm::vec3 direction{-0.125803f, 0.132744f, 0.983134f};*/
  //glm::vec3 direction = glm::normalize(dest - origin);
  ////glm::vec3 origin{ 350.0f, 548.8f, 350.0f };
  ////glm::vec3 direction= glm::normalize(glm::vec3{ -0.1f, -1.0f, -0.1f });
  //sparks::RendererSettings renderer_settings;
  //sparks::PathTracer path_tracer(&renderer_settings, &scene);
  //for (int i = 0; i < 10; i++) {
  //  path_tracer.SetSeed(i);
  //  auto& color = path_tracer.SampleRayPathTrace(origin, direction, 0.0f);
  //  //LAND_INFO("Color {}", glm::to_string(color));
  //  std::cout << "------------------" <<std::endl;
  //}

  std::mt19937 rng(std::random_device{}());
  for (int i = 0; i < 10; i++) {
    auto& a = sparks::hemisphere_sample(glm::vec3{ 0,-1,0 }, rng);
    float pdf;
    auto& b = sparks::hemisphere_sample_cosine_weighted(glm::vec3{ 0,-1,0 }, rng, &pdf);
    LAND_INFO("{}, sample {}, weighted sample {}", i, glm::to_string(a), glm::to_string(b));
  }

  //glm::vec3 displace{ 0.0f }; // displacement
  //glm::mat4 translation = glm::translate(glm::mat4{ 1.0f }, displace);
  //glm::mat4 transform{ 1.0f };
  //glm::mat4 total = translation * transform;
  //LAND_INFO("Translation {}, transformation {}, total {}", glm::to_string(translation), glm::to_string(transform), glm::to_string(total));
  //glm::mat4 inv_transform = glm::inverse(transform);
  //glm::mat4 inv_total = glm::inverse(total);
  //LAND_INFO("Inv transformation {}, inv total {}", glm::to_string(inv_transform), glm::to_string(inv_total));

}