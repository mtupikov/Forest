#pragma once

#include "ExpressionNode.h"

#include <AbstractBST.h>

#include <vector>

struct ExpressionResult {
	char varName;
	double varResult;
};

class EBST : public AbstractBST<ExpressionNode, bool> {
public:
	using AbstractBaseTree = AbstractBST<ExpressionNode, bool>;
	using AbstractBaseTree::find;

	enum class OutputType {
		Infix,
		Postfix
	};

	explicit EBST(const std::string& expressionString);

	std::string toString(OutputType type = OutputType::Infix) const;

	std::vector<ExpressionResult> calculateResult() const;

private:
	struct Node final : public AbstractBaseTree::AbstractNode {
		Node(const typename AbstractBaseTree::KVPair& keyValue);

		typename AbstractBaseTree::AbstractNode::Ptr next() const override;
	};

	using NodePtr = typename Node::Ptr;

	 void insert(const ExpressionNode& key, const bool& = false) override;
	 NodePtr insert(NodePtr& node, const typename AbstractBaseTree::KVPair& keyValue) override;

	 // unused stuff
	 bool remove(const ExpressionNode& key) override;
	 NodePtr remove(NodePtr &p, const ExpressionNode &key) override;
	 NodePtr find(const NodePtr& node, const ExpressionNode& key) const override;
};
