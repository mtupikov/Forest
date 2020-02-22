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

	enum OutputType {
		Infix = 1 << 0,
		Postfix = 1 << 1,
		Prefix = 1 << 2,
        WithParenthese = 1 << 3
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

    std::string outputInfix(const NodePtr& ptr, bool withBrackets) const;
    std::string outputPostfix(const NodePtr& ptr) const;
    std::string outputPrefix(const NodePtr& ptr) const;

    void buildTree(const std::vector<ExpressionNode>& expressionString);
    std::vector<ExpressionNode> parseExpression(const std::string& expressionString) const;

    void insert(const ExpressionNode& key, const bool&) override;
    NodePtr insert(NodePtr& node, const typename AbstractBaseTree::KVPair& keyValue) override;

	// unused stuff
	bool remove(const ExpressionNode& key) override;
	NodePtr remove(NodePtr &p, const ExpressionNode &key) override;
	NodePtr find(const NodePtr& node, const ExpressionNode& key) const override;
};

inline EBST::OutputType operator|(EBST::OutputType a, EBST::OutputType b) {
    return static_cast<EBST::OutputType>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

inline EBST::OutputType operator&(EBST::OutputType a, EBST::OutputType b) {
    return static_cast<EBST::OutputType>(static_cast<unsigned>(a) & static_cast<unsigned>(b));
}