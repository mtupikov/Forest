#pragma once

#include "ExpressionNode.h"

#include <AbstractBST.h>
#include <vector>
#include <exception>

struct ExpressionResult {
	char varName;
	double varResult;
};

class ExpressionTreeException : public std::logic_error {
public:
    ExpressionTreeException(const std::string& message, int column);

    std::string errorMessage() const;
    int column() const;

private:
    std::string m_message;
    int m_column;
};

class EBST : public AbstractBST<ExpressionNode, bool> {
public:
	using AbstractBaseTree = AbstractBST<ExpressionNode, bool>;
	using AbstractBaseTree::find;

	enum class OutputType {
		Infix,
        InfixWithParentheses,
        Postfix,
		Prefix,
        ReducedInfix,
		ReducedInfixWithParentheses
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
    void buildReducedFormTree();
    NodePtr reduceNode(const NodePtr& parent);
    std::vector<ExpressionNode> parseExpression(const std::string& expressionString) const;

	// helpers
	NodePtr allocateNode(const ExpressionNode& node) const;
	ExpressionNode getExpressionNode(const NodePtr& ptr) const;
	NodePtr calculateTwoNumbers(const NodePtr& node, const ExpressionNode& leftExp, const ExpressionNode& rightExp) const;
	bool nodeHasUnknownExpr(const NodePtr& ptr) const;
	NodePtr evaluateSubTreeWithUnknowns(const NodePtr& ptr) const;
	// helpers end

    NodePtr m_reducedTreeRootNode;

	// unused stuff
    void insert(const ExpressionNode& key, const bool&) override;
    NodePtr insert(NodePtr& node, const typename AbstractBaseTree::KVPair& keyValue) override;
    bool remove(const ExpressionNode& key) override;
	NodePtr remove(NodePtr &p, const ExpressionNode &key) override;
	NodePtr find(const NodePtr& node, const ExpressionNode& key) const override;
	iterator find(const ExpressionNode& key) const override;
	iterator begin() const override;
};
