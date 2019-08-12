#pragma once

#include <memory>

template <typename T>
class RBST {
public:
	RBST() = default;

private:
	struct Node final {
		using UPtr = std::unique_ptr<Node>;

		Node(T& key);

		T m_key;
		size_t m_height{ 1 };
		UPtr m_left;
		UPtr m_right;
	};

	size_t safeGetHeight(Node::UPtr& node) const;
	void fixHeight(Node::UPtr& node);
	Node::UPtr& find(Node::UPtr& node, const T& key) const;
	Node::UPtr& insert(Node::UPtr& node, const T& key);

	Node::UPtr m_rootNode;
	size_t m_size{ 0 };
};

template <typename T>
RBST::Node(T& key) {
	m_key = key;
};

size_t safeGetHeight(RBST::Node::UPtr& node) const {
	if (node) {
		return node->m_height;
	}

	return 0;
}

void fixHeight(Node::RBST::UPtr& node) {
	if (node) {
		node->m_height = safeGetHeight(node->m_left) + safeGetHeight(node->m_right) + 1;
	}
}

template <typename T>
RBST::Node::UPtr& find(RBST::Node::UPtr& node, const T& key) const {
	if (!node) {
		return nullptr;
	}

	if (node->m_key == key) {
		return node;
	}

	if (node->m_key > key) {
		return find(node->m_left, key);
	} else {
		return find(node->m_right, key);
	}
}

RBST::Node::UPtr& insert(RBST::Node::UPtr& node, const T& key) {
	if (!node) {
		return std::make_unique<Node>(key);
	}

	if (node->key > key) {
		node->m_left = insert(node->m_left, key);
	} else {
		node->m_right = insert(node->m_right, key);
	}
	
	fixHeight(node);

	return node;
}