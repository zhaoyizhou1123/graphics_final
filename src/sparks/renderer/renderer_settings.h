#pragma once

namespace sparks {
struct RendererSettings {
  int num_samples{1};
  int num_bounces{32};
  float prob_rr{ 0.9 }; // russian roulette probability
  float max_color{ 5.0 };
};
}  // namespace sparks
