#include "plane.h"
#include <random>

sparks::Plane::Plane(
    float coord0_min, float coord0_max, 
    float coord1_min, float coord1_max, 
    float coord2, const char* xyz_order):
    coord0_min_(coord0_min),
    coord0_max_(coord0_max),
    coord1_min_(coord1_min),
    coord1_max_(coord1_max),
    coord2_(coord2),
    xyz_2_coord_index_(std::vector<int>(3))
{
    for (int i = 0; i < 3; i++) {
        char dim = xyz_order[i];
        if (dim == 'x') {
            xyz_2_coord_index_[0] = i;
        }
        else if (dim == 'y') {
            xyz_2_coord_index_[1] = i;
        }
        else if (dim == 'z') {
            xyz_2_coord_index_[2] = i;
        }
        else {
            throw "invalid xyz_order!";
        }
    }
    float range0 = coord0_max_ - coord0_min_;
    if (range0 < 0) {
        throw "Coordinate 0 min larger than max!";
    }
    float range1 = coord1_max_ - coord1_min_;
    if (range1 < 0) {
        throw "Coordinate 1 min larger than max!";
    }
    area_ = range0 * range1;
}

float sparks::Plane::GetArea() const
{
    return area_;
}

glm::vec3 sparks::Plane::Sample(int seed0, int seed1) const
{
    std::mt19937 rng0(seed0);
    std::mt19937 rng1(seed1);
    float coord0 = std::uniform_real_distribution<float>(coord0_min_, coord0_max_)(rng0);
    float coord1 = std::uniform_real_distribution<float>(coord1_min_, coord1_max_)(rng1);

    std::vector<float> pos_index{ coord0,coord1,coord2_ };

    std::vector<float> pos_dim(3); // (x,y,z) value
    for (int dim = 0; dim < 3; dim++) { // dim corresponds to x,y,z
        pos_dim[dim] = pos_index[xyz_2_coord_index_[dim]];
    }
    return glm::vec3{pos_dim[0], pos_dim[1], pos_dim[2]};
}
