#pragma once
#include <memory>
#include <sparks/util/util.h>

namespace sparks {
// Not a convential Bst, do not compare elements
template<class ContentType> // Union of contents in inner and leaf node
class BinaryTree {
public:
	struct Node {
		Node() {
			content = nullptr;
			left_child = nullptr;
			right_child = nullptr;
		}
		Node(std::unique_ptr<ContentType> temp_content) {
			content = std::move(temp_content);
			left_child = nullptr;
			right_child = nullptr;
		}
		std::unique_ptr<ContentType> content;
		std::unique_ptr<Node> left_child;
		std::unique_ptr<Node> right_child;
	};
	// Default constructor, build a root Node with no valid content
	BinaryTree() : root_{ std::make_unique<Node>()} {}

	BinaryTree(std::unique_ptr<ContentType> content) {
		root_ = std::make_unique<Node >(std::move(content));
	}
	Node* GetRoot() {
		return root_.get();
	}
	/*@brief Add children for a node in tree
	* @param left_child, right_child: *left_child and *right_child will be modified as the pointer to new created children
	*/
	void SplitNode(
		Node* node,
		std::unique_ptr<ContentType> left_content,
		std::unique_ptr<ContentType> right_content,
		Node** left_child,
		Node** right_child) {
		if (!(node->left_child == nullptr && node->right_child == nullptr)) { // Not a current leaf
			LAND_ERROR("Node is not a leaf!");
		}
		//std::unique_ptr<Node > left_node = std::make_unique<Node >(std::move(left_content));
		//std::unique_ptr<Node > right_node = std::make_unique<Node >(std::move(right_content));
		//left_child = left_node.get();
		//right_child = right_node.get();
		node->left_child = std::make_unique<Node >(std::move(left_content));
		node->right_child = std::make_unique<Node >(std::move(right_content));
		if (node->left_child == nullptr || node->right_child == nullptr) {
			LAND_ERROR("Get nullptr for node children!");
		}
		//node->left_child = std::move(left_child);
		//node->right_child = std::move(right_child);
		*left_child = node->left_child.get();
		*right_child = node->right_child.get();
	}
private:
	std::unique_ptr<Node > root_{nullptr};
};

} // namespace sparks