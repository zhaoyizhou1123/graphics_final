#pragma once
#include "grassland/util/util.h"
#include <iostream>
#include <string>

namespace sparks {
constexpr float PI = 3.14159265358979323f;
constexpr float INV_PI = 0.31830988618379067f;

std::string PathToFilename(const std::string &file_path);

void show_vector(std::vector<int> v);

// return x**5
float pow5(float x);

template<class T>
T linear_interpolate(float t, T v1, T v2) {
  return (1 - t) * v1 + t * v2;
}

template <typename T, typename U, typename V>
inline T clamp(T val, U low, V high) {
  if (val < low)
    return low;
  else if (val > high)
    return high;
  else
    return val;
}
}  // namespace sparks
