#pragma once

namespace sparks {
struct RendererSettings {
  int num_samples{1};
  int num_bounces{16};
  float prob_rr{ 0.9 }; // russian roulette probability
};
}  // namespace sparks
