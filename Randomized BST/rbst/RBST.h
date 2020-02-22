#pragma once

#include <AbstractBST.h>

template <typename K, typename V>
class RBST : public AbstractBST<K, V> {
public:
	using AbstractBaseTree = AbstractBST<K, V>;
	using AbstractBaseTree::find;

	RBST() = default;

	void insert(const K& key, const V& value) override;
	
	bool remove(const K& key) override;

	void printTree() const;

private:
	struct Node final : public AbstractBaseTree::AbstractNode {
		Node(const typename AbstractBaseTree::KVPair& keyValue);

		typename AbstractBaseTree::AbstractNode::Ptr next() const override;
	};

	using NodePtr = typename Node::Ptr;

	void printBinaryTree(const std::string& prefix, const typename Node::Ptr& node, bool isLeft) const;

	size_t safeGetSize(const NodePtr& node) const;
	void fixSize(NodePtr& node);

	NodePtr find(const NodePtr& node, const K& key) const override;

	NodePtr insert(NodePtr& node, const typename AbstractBaseTree::KVPair& keyValue) override;
	NodePtr insertRoot(NodePtr& node, const typename AbstractBaseTree::KVPair& keyValue);

	NodePtr rotateRight(NodePtr& node);
	NodePtr rotateLeft(NodePtr& node);

	NodePtr join(NodePtr& p, NodePtr& q);

	NodePtr remove(NodePtr& p, const K& key) override;
};

template <typename K, typename V>
void RBST<K, V>::insert(const K& key, const V& value) {
	this->m_rootNode = insert(this->m_rootNode, std::make_pair(key, value));
	++this->m_size;
}

template <typename K, typename V>
bool RBST<K, V>::remove(const K& key) {
	if (!this->contains(key)) {
		return false;
	}

	this->m_rootNode = remove(this->m_rootNode, key);
	--this->m_size;

	return true;
}

template <typename K, typename V>
void RBST<K, V>::printTree() const {
	printBinaryTree("", this->m_rootNode, false);
}

template <typename K, typename V>
RBST<K, V>::Node::Node(const typename AbstractBaseTree::KVPair& keyValue) :
    AbstractBaseTree::AbstractNode(keyValue)
{
}

template <typename K, typename V>
typename RBST<K, V>::AbstractBaseTree::AbstractNode::Ptr RBST<K, V>::Node::next() const {
	typename AbstractBaseTree::AbstractNode::Ptr ptr;
	auto strongParent = this->m_parent.lock();

	if (!strongParent) {
		return ptr;
	}

	if ((this == strongParent->m_left.get()) && (strongParent->m_right)) {
		ptr = strongParent->m_right;
	} else {
		return strongParent;
	}

	while (true) {
		if (ptr->m_left) {
			ptr = ptr->m_left;
		} else if (ptr->m_right) {
			ptr = ptr->m_right;
		} else {
			break;
		}
	}

	return ptr;
}

template <typename K, typename V>
void RBST<K, V>::printBinaryTree(const std::string& prefix, const NodePtr& node, bool isLeft) const {
	if (node) {
		std::string parentStr;
		const auto& strongParent = node->m_parent.lock();
		if (strongParent) {
			parentStr = " (parent : ";
			parentStr += std::to_string(strongParent->m_keyValue.first);
			parentStr += ")";
		}

		std::cout	<< prefix.c_str()
		            << (isLeft ? "├──" : "└──" )
		            << " (" << node->m_keyValue.first
		            << ", " << node->m_keyValue.second << ") "
		            <<  parentStr << std::endl;

		printBinaryTree(prefix + (isLeft ? "│   " : "    "), node->m_left, true);
		printBinaryTree(prefix + (isLeft ? "│   " : "    "), node->m_right, false);
	}
}

template <typename K, typename V>
size_t RBST<K, V>::safeGetSize(const typename RBST<K, V>::Node::Ptr& node) const {
	if (node) {
		return node->m_size;
	}

	return 0;
}

template <typename K, typename V>
void RBST<K, V>::fixSize(typename RBST<K, V>::Node::Ptr& node) {
	if (node) {
		node->m_size = safeGetSize(node->m_left) + safeGetSize(node->m_right) + 1;
	}
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::find(const NodePtr& node, const K& key) const {
	if (!node || node->m_keyValue.first == key) {
		return node;
	}

	if (node->m_keyValue.first > key) {
		return find(node->m_left, key);
	} else {
		return find(node->m_right, key);
	}
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::insert(NodePtr& node, const typename AbstractBaseTree::KVPair& keyValue) {
	if (!node) {
		return std::make_shared<RBST<K, V>::Node>(keyValue);
	}

	std::random_device rndDev;
	if ((rndDev() % (node->m_size + 1)) == 0) {
		return insertRoot(node, keyValue);
	}

	if (node->m_keyValue.first > keyValue.first) {
		node->m_left = insert(node->m_left, keyValue);
		node->m_left->m_parent = node;
	} else {
		node->m_right = insert(node->m_right, keyValue);
		node->m_right->m_parent = node;
	}

	fixSize(node);

	return node;
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::insertRoot(NodePtr& node, const typename AbstractBaseTree::KVPair& keyValue) {
	if (!node) {
		return std::make_shared<RBST<K, V>::Node>(keyValue);
	}

	if (node->m_keyValue.first > keyValue.first) {
		node->m_left = insertRoot(node->m_left, keyValue);
		node->m_left->m_parent = node;
		return rotateRight(node);
	} else {
		node->m_right = insertRoot(node->m_right, keyValue);
		node->m_right->m_parent = node;
		return rotateLeft(node);
	}
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::rotateRight(NodePtr& node) {
	auto q = node->m_left;

	if (!q) {
		return node;
	}

	q->m_parent = node->m_parent;
	node->m_left = q->m_right;
	if (node->m_left) {
		node->m_left->m_parent = node;
	}
	q->m_right = node;
	node->m_parent = q;
	q->m_size = node->m_size;

	fixSize(node);

	return q;
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::rotateLeft(NodePtr& node) {
	auto p = node->m_right;

	if (!p) {
		return node;
	}

	p->m_parent = node->m_parent;
	node->m_right = p->m_left;
	if (node->m_right) {
		node->m_right->m_parent = node;
	}
	p->m_left = node;
	node->m_parent = p;
	p->m_size = node->m_size;

	fixSize(node);

	return p;
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::join(NodePtr& p, NodePtr& q) {
	if (!p) {
		return q;
	}

	if (!q) {
		return p;
	}

	std::random_device rndDev;
	if ((rndDev() % (p->m_size + q->m_size)) < p->m_size) {
		p->m_right = join(p->m_right, q);
		p->m_right->m_parent = p;
		fixSize(p);
		return p;
	} else {
		q->m_left = join(p, q->m_left);
		q->m_left->m_parent = q;
		fixSize(q);
		return q;
	}
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::remove(NodePtr& node, const K& key) {
	if (!node) {
		return node;
	}

	if (node->m_keyValue.first == key) {
		auto q = join(node->m_left, node->m_right);
		node.reset();
		return q;
	}

	if (node->m_keyValue.first > key && (node->m_left = remove(node->m_left, key))) {
		node->m_left->m_parent = node;
	} else if ((node->m_right = remove(node->m_right, key))) {
		node->m_right->m_parent = node;
	}

	return node;
}
