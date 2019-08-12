#pragma once

#include <memory>
#include <random>

template <typename T>
class RBST {
public:
	RBST() = default;

private:
	struct Node final {
		using Ptr = std::shared_ptr<Node>;

		Node(T& key);

		T m_key;
		size_t m_height{ 1 };
		Ptr m_left;
		Ptr m_right;
	};

	size_t safeGetHeight(const Node::Ptr& node) const;
	void fixHeight(Node::Ptr& node);

	Node::Ptr find(Node::Ptr& node, const T& key) const;

	Node::Ptr insert(Node::Ptr& node, const T& key);
	Node::Ptr insertRoot(Node::Ptr& node, const T& key);

	Node::Ptr rotateRight(Node::Ptr& node);
	Node::Ptr rotateLeft(Node::Ptr& node);

	Node::Ptr join(Node::Ptr& p, Node::Ptr& q);

	Node::Ptr remove(Node::Ptr& p, const T& key);

	Node::Ptr m_rootNode;
	size_t m_size{ 0 };
};

template <typename T>
RBST::Node(T& key) {
	m_key = key;
};

size_t safeGetHeight(const RBST::Node::Ptr& node) const {
	if (node) {
		return node->m_height;
	}

	return 0;
}

void fixHeight(Node::RBST::Ptr& node) {
	if (node) {
		node->m_height = safeGetHeight(node->m_left) + safeGetHeight(node->m_right) + 1;
	}
}

template <typename T>
RBST::Node::Ptr find(RBST::Node::Ptr& node, const T& key) const {
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

template <typename T>
RBST::Node::Ptr insert(RBST::Node::Ptr& node, const T& key) {
	if (!node) {
		return std::make_shared<RBST::Node>(key);
	}

	std::random_device rndDev;
	if ((rndDev() % node->m_height + 1) == 0) {
		return insertRoot(node, key);
	}

	if (node->key > key) {
		node->m_left = insert(node->m_left, key);
	} else {
		node->m_right = insert(node->m_right, key);
	}
	
	fixHeight(node);

	return node;
}

template <typename T>
RBST::Node::Ptr insertRoot(RBST::Node::Ptr& node, const T& key) {
	if (!node) {
		return std::make_shared<RBST::Node>(key);
	}

	if (node->m_key > key) {
		node->m_left = insertRoot(node->m_left, key);
		return rotateRight(node);
	} else {
		node->m_right = insertRoot(node->m_right, key);
		return rotateLeft(node);
	}
}

RBST::Node::Ptr rotateRight(RBST::Node::Ptr& node) {
	auto q = node->m_left;

	if (!q) {
		return node;
	}

	node->m_left = q->m_right;
	q->m_right = node;
	q->m_height = node->m_height;

	fixHeight(node);

	return q;
}

RBST::Node::Ptr rotateLeft(RBST::Node::Ptr& node) {
	auto p = node->m_right;

	if (!p) {
		return node;
	}

	node->m_right = p->m_left;
	p->m_left = node;
	p->m_height = node->m_height;

	fixHeight(node);

	return p;
}
RBST::Node::Ptr join(RBST::Node::Ptr& p, RBST::Node::Ptr& q) {
	if (!p) {
		return q;
	}

	if (!q) {
		return p;
	}

	std::random_device rndDev;
	if ((rndDev() % (p->m_height + q->m_height)) < p->m_height) {
		p->m_right = join(p->m_right, q);
		fixHeight(p);
		return p;
	} else {
		p->m_left = join(p, q->m_left);
		fixHeight(q);
		return q;
	}
}

template <typename T>
RBST::Node::Ptr remove(RBST::Node::Ptr& node, const T& key) {
	if (!node) {
		return node;
	}

	if (node->m_key == key) {
		auto q = join(node->m_left, node->m_right);
		node.reset();
		return q;
	}

	if (node->m_key > key) {
		node->m_left = remove(node->m_left, key);
	} else {
		node->m_right = remove(node->m_right, key);
	}
	return node;
}
