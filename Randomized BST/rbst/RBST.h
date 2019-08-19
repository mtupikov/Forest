#pragma once

#include <memory>
#include <random>
#include <utility>
#include <assert.h>
#include <iostream>
#include <iterator>

template <typename K, typename V>
class RBST {
	struct Node;

public:
	class NodeIterator;

	using iterator = NodeIterator;
	using const_iterator = const NodeIterator;
	using KVPair = std::pair<K, V>;

	RBST() = default;

	bool contains(const K& key) const;

	V& find(const K& key) const;

	void insert(const K& key, const V& value);
	
	bool remove(const K& key);

	void clear();

	size_t size() const;

	void printTree() const;

	class NodeIterator final : public std::iterator<std::bidirectional_iterator_tag, KVPair> {
	public:
		using pointer = typename std::iterator<std::bidirectional_iterator_tag, KVPair>::pointer;

		NodeIterator(pointer ptr);
		NodeIterator(const NodeIterator& other);

		NodeIterator& operator++();
		NodeIterator operator++(int);

		NodeIterator& operator--();
		NodeIterator operator--(int);

		bool operator==(const NodeIterator& other) const;
		bool operator!=(const NodeIterator& other) const;

		pointer operator*();

	private:
		typename Node::Ptr m_item;
	};

private:
	struct Node final {
		using Ptr = std::shared_ptr<Node>;
		using WPtr = std::weak_ptr<Node>;

		Node(const KVPair& keyValue) {
			m_keyValue = keyValue;
		}

		KVPair m_keyValue;
		size_t m_height{ 1 };
		WPtr m_parent;
		Ptr m_left;
		Ptr m_right;
	};

	void printBinaryTree(const std::string& prefix, const typename Node::Ptr& node, bool isLeft) const;

	size_t safeGetHeight(const typename Node::Ptr& node) const;
	void fixHeight(typename Node::Ptr& node);

	typename Node::Ptr find(const typename Node::Ptr& node, const K& key) const;

	typename Node::Ptr insert(typename Node::Ptr& node, const KVPair& keyValue);
	typename Node::Ptr insertRoot(typename Node::Ptr& node, const KVPair& keyValue);

	typename Node::Ptr rotateRight(typename Node::Ptr& node);
	typename Node::Ptr rotateLeft(typename Node::Ptr& node);

	typename Node::Ptr join(typename Node::Ptr& p, typename Node::Ptr& q);

	typename Node::Ptr remove(typename Node::Ptr& p, const K& key);

	typename Node::Ptr m_rootNode;
	size_t m_size{ 0 };
};

template <typename K, typename V>
bool RBST<K, V>::contains(const K& key) const {
	return find(m_rootNode, key) != nullptr;
}

template <typename K, typename V>
void RBST<K, V>::insert(const K& key, const V& value) {
	m_rootNode = insert(m_rootNode, std::make_pair(key, value));
	++m_size;
}

template <typename K, typename V>
V& RBST<K, V>::find(const K& key) const {
	auto ptr = find(m_rootNode, key);

	assert(ptr != nullptr); // remove when iterators are implemented

	return ptr->m_keyValue.second;
}

template <typename K, typename V>
bool RBST<K, V>::remove(const K& key) {
	auto ptr = remove(m_rootNode, key);

	m_rootNode = ptr;
	--m_size;

	return true; // refactor when iterators are implemented
}

template <typename K, typename V>
void RBST<K, V>::clear() {
	m_rootNode.reset();
	m_size = 0;
}

template <typename K, typename V>
size_t RBST<K, V>::size() const {
	return m_size;
}

template <typename K, typename V>
void RBST<K, V>::printTree() const {
	printBinaryTree("", m_rootNode, false);
}

template <typename K, typename V>
void RBST<K, V>::printBinaryTree(const std::string& prefix, const typename Node::Ptr& node, bool isLeft) const {
	if (node) {
		std::string parentStr;
		const auto& strongParent = node->m_parent.lock();
		if (strongParent) {
			parentStr = " (parent : ";
			parentStr += strongParent->m_keyValue.second;
			parentStr += ")";
		}

		std::cout	<< prefix.c_str()
		            << (isLeft ? "├──" : "└──" )
		            << " " << node->m_keyValue.first
		            <<  parentStr << std::endl;

		printBinaryTree(prefix + (isLeft ? "│   " : "    "), node->m_left, true);
		printBinaryTree(prefix + (isLeft ? "│   " : "    "), node->m_right, false);
	}
}

template <typename K, typename V>
size_t RBST<K, V>::safeGetHeight(const typename RBST<K, V>::Node::Ptr& node) const {
	if (node) {
		return node->m_height;
	}

	return 0;
}

template <typename K, typename V>
void RBST<K, V>::fixHeight(typename RBST<K, V>::Node::Ptr& node) {
	if (node) {
		node->m_height = safeGetHeight(node->m_left) + safeGetHeight(node->m_right) + 1;
	}
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::find(const typename RBST<K, V>::Node::Ptr& node, const K& key) const {
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
typename RBST<K, V>::Node::Ptr RBST<K, V>::insert(typename RBST<K, V>::Node::Ptr& node, const KVPair& keyValue) {
	if (!node) {
		return std::make_shared<RBST<K, V>::Node>(keyValue);
	}

	std::random_device rndDev;
	if ((rndDev() % (node->m_height + 1)) == 0) {
		return insertRoot(node, keyValue);
	}

	if (node->m_keyValue.first > keyValue.first) {
		node->m_left = insert(node->m_left, keyValue);
		node->m_left->m_parent = node;
	} else {
		node->m_right = insert(node->m_right, keyValue);
		node->m_right->m_parent = node;
	}

	fixHeight(node);

	return node;
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::insertRoot(typename RBST<K, V>::Node::Ptr& node, const KVPair& keyValue) {
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
typename RBST<K, V>::Node::Ptr RBST<K, V>::rotateRight(typename RBST<K, V>::Node::Ptr& node) {
	auto q = node->m_left;

	if (!q) {
		return node;
	}

	q->m_parent = node->m_parent; //
	node->m_left = q->m_right;
	if (node->m_left) {
		node->m_left->m_parent = node; //
	}
	q->m_right = node;
	node->m_parent = q; //
	q->m_height = node->m_height;

	fixHeight(node);

	return q;
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::rotateLeft(typename RBST<K, V>::Node::Ptr& node) {
	auto p = node->m_right;

	if (!p) {
		return node;
	}

	p->m_parent = node->m_parent; //
	node->m_right = p->m_left;
	if (node->m_right) {
		node->m_right->m_parent = node; //
	}
	p->m_left = node;
	node->m_parent = p; //
	p->m_height = node->m_height;

	fixHeight(node);

	return p;
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::join(typename RBST<K, V>::Node::Ptr& p, typename RBST<K, V>::Node::Ptr& q) {
	if (!p) {
		return q;
	}

	if (!q) {
		return p;
	}

	std::random_device rndDev;
	if ((rndDev() % (p->m_height + q->m_height)) < p->m_height) {
		p->m_right = join(p->m_right, q);
		p->m_right->m_parent = p;
		fixHeight(p);
		return p;
	} else {
		q->m_left = join(p, q->m_left);
		q->m_left->m_parent = q;
		fixHeight(q);
		return q;
	}
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::remove(typename RBST<K, V>::Node::Ptr& node, const K& key) {
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
