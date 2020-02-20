#pragma once

#include <memory>
#include <random>
#include <utility>
#include <assert.h>
#include <iostream>
#include <iterator>
#include <string>

template <typename K, typename V, typename IteratorTag = std::forward_iterator_tag>
class AbstractBST {
protected:
	struct AbstractNode;

	template <bool enabled>
	struct NextNodeInterface;

	template <>
	struct NextNodeInterface<false> {};

	template <>
	struct NextNodeInterface<true> {
		virtual std::shared_ptr<AbstractNode> prev() const = 0;
	};

public:
	class NodeIterator;

	using iterator = NodeIterator;
	using const_iterator = const NodeIterator;
	using KVPair = std::pair<K, V>;

	AbstractBST() = default;

	bool contains(const K& key) const;

	virtual iterator find(const K& key) const;

	virtual void insert(const K& key, const V& value) = 0;
	
	virtual bool remove(const K& key) = 0;

	void clear();

	size_t size() const;

	virtual iterator begin() const;
	const_iterator cbegin() const;

	iterator end() const;
	const_iterator cend() const;

	class NodeIterator final : public std::iterator<IteratorTag, typename AbstractNode::Ptr> {
	public:
		NodeIterator() = default;
		NodeIterator(typename AbstractNode::Ptr ptr);
		NodeIterator(const NodeIterator& other);

		NodeIterator& operator++();
		NodeIterator operator++(int);

		template <typename = typename std::enable_if<std::is_same<IteratorTag, std::bidirectional_iterator_tag>::value>>
		NodeIterator& operator--();
		template <typename = typename std::enable_if<std::is_same<IteratorTag, std::bidirectional_iterator_tag>::value>>
		NodeIterator operator--(int);

		bool operator==(const NodeIterator& other) const;
		bool operator!=(const NodeIterator& other) const;

		KVPair& operator*();
		const KVPair& operator*() const;

		KVPair* operator->();
		const KVPair* operator->() const;

		operator bool() const;

	private:
		typename AbstractNode::Ptr m_item;
	};

protected:
	struct AbstractNode : public NextNodeInterface<std::is_same<IteratorTag, std::bidirectional_iterator_tag>::value> {
		using Ptr = std::shared_ptr<AbstractNode>;
		using WPtr = std::weak_ptr<AbstractNode>;

		AbstractNode(const KVPair& keyValue);

		virtual Ptr next() const = 0;

		KVPair m_keyValue;
		size_t m_size{ 1 };
		WPtr m_parent;
		Ptr m_left;
		Ptr m_right;
	};

	using NodePtr = typename AbstractNode::Ptr;

	NodePtr mostLeftNode() const;

	virtual NodePtr find(const NodePtr& node, const K& key) const = 0;

	virtual NodePtr insert(NodePtr& node, const KVPair& keyValue) = 0;

	virtual NodePtr remove(NodePtr& p, const K& key) = 0;

	NodePtr m_rootNode;
	size_t m_size{ 0 };
};

template <typename K, typename V, typename IteratorTag>
bool AbstractBST<K, V, IteratorTag>::contains(const K& key) const {
	return find(m_rootNode, key) != nullptr;
}

template <typename K, typename V, typename IteratorTag>
typename AbstractBST<K, V, IteratorTag>::iterator AbstractBST<K, V, IteratorTag>::find(const K& key) const {
	auto ptr = find(m_rootNode, key);

	return iterator(ptr);
}

template <typename K, typename V, typename IteratorTag>
void AbstractBST<K, V, IteratorTag>::clear() {
	m_rootNode.reset();
	m_size = 0;
}

template <typename K, typename V, typename IteratorTag>
size_t AbstractBST<K, V, IteratorTag>::size() const {
	return m_size;
}

template <typename K, typename V, typename IteratorTag>
typename AbstractBST<K, V, IteratorTag>::iterator AbstractBST<K, V, IteratorTag>::begin() const {
	return iterator(mostLeftNode());
}

template <typename K, typename V, typename IteratorTag>
const typename AbstractBST<K, V, IteratorTag>::iterator AbstractBST<K, V, IteratorTag>::cbegin() const {
	return begin();
}

template <typename K, typename V, typename IteratorTag>
typename AbstractBST<K, V, IteratorTag>::iterator AbstractBST<K, V, IteratorTag>::end() const {
	return iterator();
}

template <typename K, typename V, typename IteratorTag>
const typename AbstractBST<K, V, IteratorTag>::iterator AbstractBST<K, V, IteratorTag>::cend() const {
	return const_iterator();
}

template <typename K, typename V, typename IteratorTag>
AbstractBST<K, V, IteratorTag>::NodeIterator::NodeIterator(NodePtr ptr) {
	m_item = ptr;
}

template <typename K, typename V, typename IteratorTag>
AbstractBST<K, V, IteratorTag>::NodeIterator::NodeIterator(const typename AbstractBST<K, V, IteratorTag>::NodeIterator& other) {
	*this = other;
}

template <typename K, typename V, typename IteratorTag>
typename AbstractBST<K, V, IteratorTag>::NodeIterator& AbstractBST<K, V, IteratorTag>::NodeIterator::operator++() {
	m_item = m_item->next();
	return *this;
}

template <typename K, typename V, typename IteratorTag>
typename AbstractBST<K, V, IteratorTag>::NodeIterator AbstractBST<K, V, IteratorTag>::NodeIterator::operator++(int) {
	auto tmp = *this;
	operator++();
	return tmp;
}

template <typename K, typename V, typename IteratorTag> template <typename >
typename AbstractBST<K, V, IteratorTag>::NodeIterator& AbstractBST<K, V, IteratorTag>::NodeIterator::operator--() {
	m_item = m_item->prev();
	return *this;
}

template <typename K, typename V, typename IteratorTag> template <typename >
typename AbstractBST<K, V, IteratorTag>::NodeIterator AbstractBST<K, V, IteratorTag>::NodeIterator::operator--(int) {
	auto tmp = *this;
	operator--();
	return tmp;
}

template <typename K, typename V, typename IteratorTag>
bool AbstractBST<K, V, IteratorTag>::NodeIterator::operator==(const NodeIterator& other) const {
	if (!m_item && !other.m_item) {
		return true;
	}

	if (!other.m_item || !m_item) {
		return false;
	}

	return m_item->m_keyValue == other.m_item->m_keyValue;
}

template <typename K, typename V, typename IteratorTag>
bool AbstractBST<K, V, IteratorTag>::NodeIterator::operator!=(const NodeIterator& other) const {
	return !(*this == other);
}

template <typename K, typename V, typename IteratorTag>
typename AbstractBST<K, V, IteratorTag>::KVPair& AbstractBST<K, V, IteratorTag>::NodeIterator::operator*() {
	return m_item->m_keyValue;
}

template <typename K, typename V, typename IteratorTag>
const typename AbstractBST<K, V, IteratorTag>::KVPair& AbstractBST<K, V, IteratorTag>::NodeIterator::operator*() const {
	return m_item->m_keyValue;
}

template <typename K, typename V, typename IteratorTag>
typename AbstractBST<K, V, IteratorTag>::KVPair* AbstractBST<K, V, IteratorTag>::NodeIterator::operator->() {
	return &m_item->m_keyValue;
}

template <typename K, typename V, typename IteratorTag>
const typename AbstractBST<K, V, IteratorTag>::KVPair* AbstractBST<K, V, IteratorTag>::NodeIterator::operator->() const {
	return &m_item->m_keyValue;
}

template <typename K, typename V, typename IteratorTag>
AbstractBST<K, V, IteratorTag>::NodeIterator::operator bool() const {
	return m_item != nullptr;
}

template <typename K, typename V, typename IteratorTag>
AbstractBST<K, V, IteratorTag>::AbstractNode::AbstractNode(const KVPair& keyValue) {
	m_keyValue = keyValue;
}

template<typename K, typename V, typename IteratorTag>
typename AbstractBST<K, V, IteratorTag>::NodePtr AbstractBST<K, V, IteratorTag>::mostLeftNode() const
{
	auto ptr = m_rootNode;
	while (ptr && ptr->m_left) {
		ptr = ptr->m_left;
	}

	return ptr;
}
