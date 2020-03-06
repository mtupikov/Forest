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

	enum NodeRule {
		Subtree = 1 << 0,
		UnknownVar = 1 << 1,
		NumberVar = 1 << 2,
		UnknownAndSubtree = 1 << 3,
		NumberAndSubtree = 1 << 4,

		Multiplication = 1 << 5,
		AdditionSubstitution = 1 << 6,

		NoRule = 1 << 10,

		UnknownAndSubtreeMul = UnknownAndSubtree | Multiplication,
		UnknownAndSubtreeAddSub = UnknownAndSubtree | AdditionSubstitution,
		NumberAndSubtreeMul = NumberAndSubtree | Multiplication,
		NumberAndSubtreeAddSub = NumberAndSubtree | AdditionSubstitution,

		Rule1 = Subtree | Multiplication | UnknownAndSubtreeMul, // ((x * A) * B) -> (x * (A * B))
		Rule2 = Subtree | Multiplication | NumberAndSubtreeMul, // ((n * A) * B) -> (n * (A * B))
		Rule3 = UnknownAndSubtreeMul | UnknownAndSubtreeMul | AdditionSubstitution, // ((x * A) + (x * B)) -> (x * (A + B))
		Rule4 = NumberAndSubtreeMul | NumberAndSubtreeMul | AdditionSubstitution, // ((n * A) + (n * B)) -> (n * (A + B))
		Rule5 = Subtree | UnknownAndSubtreeAddSub | AdditionSubstitution, // (A + (x + B)) -> (x + (A + B))
		Rule6 = Subtree | NumberAndSubtreeAddSub | AdditionSubstitution, // (A + (n + B)) -> (n + (A + B))
	};

	friend NodeRule operator|(NodeRule a, NodeRule b);

	bool nodeHasChildren(const NodePtr& node) const;

	NodeRule getRuleForSubtree(const NodePtr& node) const;
	NodeRule getRuleForNode(const NodePtr& node) const;

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
	NodePtr evaluateOperatorAndNumber(NodePtr& node, const ExpressionNode& number, bool leftIsOp) const;

	NodePtr applyRulesToSubTree(NodePtr& parent) const;

	bool subTreeIsUnknownWithNumber(const NodePtr& node) const;
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
