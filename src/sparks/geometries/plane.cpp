#include "plane.h"
#include <string>
#include "grassland/util/logging.h"

sparks::Plane::Plane(const tinyxml2::XMLElement* element)
{
    auto x = element->FirstChildElement("x");
    x_min_ = std::stof(x->FindAttribute("min")->Value());
    x_max_ = std::stof(x->FindAttribute("max")->Value());
    auto y = element->FirstChildElement("y");
    y_min_ = std::stof(y->FindAttribute("min")->Value());
    y_max_ = std::stof(y->FindAttribute("max")->Value());
    auto z = element->FirstChildElement("z");
    z_min_ = std::stof(z->FindAttribute("min")->Value());
    z_max_ = std::stof(z->FindAttribute("max")->Value());

    int degenerate_cnt = 0;
    float x_range = x_max_ - x_min_;
    if (x_range < 0) {
        throw "Plane x coordinate max < min!";
    }
    else if (x_range == 0) {
        x_range = 1.0f;
        degenerate_cnt++;
    }
    float y_range = y_max_ - y_min_;
    if (y_range < 0) {
        throw "Plane y coordinate max < min!";
    }
    else if (y_range == 0) {
        y_range = 1.0f;
        degenerate_cnt++;
    }
    float z_range = z_max_ - z_min_;
    if (z_range < 0) {
        throw "Plane z coordinate max < min!";
    }
    else if (z_range == 0) {
        z_range = 1.0f;
        degenerate_cnt++;
    }
    if (degenerate_cnt != 1) {
        throw "The plane's degenerate dim is not 0!";
    }
   
    area_ = x_range * y_range * z_range;
}

float sparks::Plane::GetArea() const
{
    return area_;
}

glm::vec3 sparks::Plane::Sample(std::mt19937& rng) const
{
    float pos_x, pos_y, pos_z;
    if (x_min_ == x_max_) {
        pos_x = x_min_;
    }
    else {
        pos_x = std::uniform_real_distribution<float>(x_min_, x_max_)(rng);
    }
    if (y_min_ == y_max_) {
        pos_y = y_min_;
    }
    else {
        pos_y = std::uniform_real_distribution<float>(y_min_, y_max_)(rng);
    }
    if (z_min_ == z_max_) {
        pos_z = z_min_;
    }
    else {
        pos_z = std::uniform_real_distribution<float>(z_min_, z_max_)(rng);
    }
    return glm::vec3{pos_x, pos_y, pos_z};
}
