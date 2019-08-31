#pragma once

#include <memory>
#include <random>
#include <utility>
#include <assert.h>
#include <iostream>
#include <iterator>
#include <string>

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

	iterator find(const K& key) const;

	void insert(const K& key, const V& value);
	
	bool remove(const K& key);

	void clear();

	size_t size() const;

	void printTree() const;

	iterator begin() const;
	const_iterator cbegin() const;

	iterator end() const;
	const_iterator cend() const;

	class NodeIterator final : public std::iterator<std::bidirectional_iterator_tag, typename Node::Ptr> {
	public:
		NodeIterator() = default;
		NodeIterator(typename Node::Ptr ptr);
		NodeIterator(const NodeIterator& other);

		NodeIterator& operator++();
		NodeIterator operator++(int);

		bool operator==(const NodeIterator& other) const;
		bool operator!=(const NodeIterator& other) const;

		KVPair& operator*();
		const KVPair& operator*() const;

		KVPair* operator->();
		const KVPair* operator->() const;

		operator bool() const;

	private:
		typename Node::Ptr m_item;
	};

private:
	struct Node final {
		using Ptr = std::shared_ptr<Node>;
		using WPtr = std::weak_ptr<Node>;

		Node(const KVPair& keyValue);

		Ptr next() const;

		KVPair m_keyValue;
		size_t m_size{ 1 };
		WPtr m_parent;
		Ptr m_left;
		Ptr m_right;
	};

	using NodePtr = typename Node::Ptr;

	void printBinaryTree(const std::string& prefix, const typename Node::Ptr& node, bool isLeft) const;

	size_t safeGetSize(const NodePtr& node) const;
	void fixHeight(NodePtr& node);

	NodePtr find(const NodePtr& node, const K& key) const;

	NodePtr insert(NodePtr& node, const KVPair& keyValue);
	NodePtr insertRoot(NodePtr& node, const KVPair& keyValue);

	NodePtr rotateRight(NodePtr& node);
	NodePtr rotateLeft(NodePtr& node);

	NodePtr join(NodePtr& p, NodePtr& q);

	NodePtr remove(NodePtr& p, const K& key);

	NodePtr m_rootNode;
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
typename RBST<K, V>::iterator RBST<K, V>::find(const K& key) const {
	auto ptr = find(m_rootNode, key);

	return iterator(ptr);
}

template <typename K, typename V>
bool RBST<K, V>::remove(const K& key) {
	if (!contains(key)) {
		return false;
	}

	m_rootNode = remove(m_rootNode, key);
	--m_size;

	return true;
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
typename RBST<K, V>::iterator RBST<K, V>::begin() const {
	auto node = m_rootNode;

	if (!node) {
		return iterator();
	}

	while (true) {
		if (node->m_left) {
			node = node->m_left;
		} else if (node->m_right) {
			node = node->m_right;
		} else {
			break;
		}
	}

	return iterator(node);
}

template <typename K, typename V>
const typename RBST<K, V>::iterator RBST<K, V>::cbegin() const {
	return begin();
}

template <typename K, typename V>
typename RBST<K, V>::iterator RBST<K, V>::end() const {
	return iterator();
}

template <typename K, typename V>
const typename RBST<K, V>::iterator RBST<K, V>::cend() const {
	return const_iterator();
}

template <typename K, typename V>
RBST<K, V>::NodeIterator::NodeIterator(NodePtr ptr) {
	m_item = ptr;
}

template <typename K, typename V>
RBST<K, V>::NodeIterator::NodeIterator(const typename RBST<K, V>::NodeIterator& other) {
	*this = other;
}

template <typename K, typename V>
typename RBST<K, V>::NodeIterator& RBST<K, V>::NodeIterator::operator++() {
	m_item = m_item->next();
	return *this;
}

template <typename K, typename V>
typename RBST<K, V>::NodeIterator RBST<K, V>::NodeIterator::operator++(int) {
	auto tmp = *this;
	operator++();
	return tmp;
}

template <typename K, typename V>
bool RBST<K, V>::NodeIterator::operator==(const NodeIterator& other) const {
	if (!m_item && !other.m_item) {
		return true;
	}

	if (!other.m_item || !m_item) {
		return false;
	}

	return m_item->m_keyValue == other.m_item->m_keyValue;
}

template <typename K, typename V>
bool RBST<K, V>::NodeIterator::operator!=(const NodeIterator& other) const {
	return !(*this == other);
}

template <typename K, typename V>
typename RBST<K, V>::KVPair& RBST<K, V>::NodeIterator::operator*() {
	return m_item->m_keyValue;
}

template <typename K, typename V>
const typename RBST<K, V>::KVPair& RBST<K, V>::NodeIterator::operator*() const {
	return m_item->m_keyValue;
}

template <typename K, typename V>
typename RBST<K, V>::KVPair* RBST<K, V>::NodeIterator::operator->() {
	return &m_item->m_keyValue;
}

template <typename K, typename V>
const typename RBST<K, V>::KVPair* RBST<K, V>::NodeIterator::operator->() const {
	return &m_item->m_keyValue;
}

template <typename K, typename V>
RBST<K, V>::NodeIterator::operator bool() const {
	return m_item != nullptr;
}

template <typename K, typename V>
RBST<K, V>::Node::Node(const KVPair& keyValue) {
	m_keyValue = keyValue;
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::Node::next() const {
	Node::Ptr ptr;
	auto strongParent = m_parent.lock();

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
void RBST<K, V>::fixHeight(typename RBST<K, V>::Node::Ptr& node) {
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
typename RBST<K, V>::Node::Ptr RBST<K, V>::insert(NodePtr& node, const KVPair& keyValue) {
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

	fixHeight(node);

	return node;
}

template <typename K, typename V>
typename RBST<K, V>::Node::Ptr RBST<K, V>::insertRoot(NodePtr& node, const KVPair& keyValue) {
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

	fixHeight(node);

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

	fixHeight(node);

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
