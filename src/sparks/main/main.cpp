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

void test_main() {
  return;
}