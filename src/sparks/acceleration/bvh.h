#pragma once
#include "sparks/assets/aabb.h"
#include <memory>
#include <vector>

namespace sparks {
// Data for Bvh tree nodes
struct BvhNodeData {
	BvhNodeData(): 
		box{},
		face_indices{},
		is_leaf{ false } {}
	BvhNodeData(const AxisAlignedBoundingBox& box_temp) :
		box{box_temp},
		face_indices{},
		is_leaf{ false } {}
	BvhNodeData(
		const AxisAlignedBoundingBox & box_temp,
		const std::vector<int> & face_indices_temp,
		bool is_leaf_temp) :
		box{ box_temp },
		face_indices{ face_indices_temp },
		is_leaf{ is_leaf_temp } {}

	AxisAlignedBoundingBox box; // bounding box
	std::vector<int> face_indices; // list of indices of faces for leaf node. Empty for internal node
	bool is_leaf; // True if this node is leaf
};
} // namespace sparks