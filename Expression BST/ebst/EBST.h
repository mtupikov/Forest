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

	struct SubtreeWithOperator {
		NodePtr subtree;
		OperatorType op;
	};

	enum NodeRule {
		Subtree = 1 << 0, // A
		UnknownVar = 1 << 1, // x
		NumberVar = 1 << 2, // n
		Multiplication = 1 << 3, // *
		AdditionSubstitution = 1 << 4, // +-
		DivisionModulo = 1 << 5, // /%
		Power = 1 << 6, // ^

		NoRule = 1 << 10,

		UnknownAndNumberMul = UnknownVar | NumberVar | Multiplication, // (x * n)
		UnknownAndNumberAddSub = UnknownVar | NumberVar | AdditionSubstitution, // (x +- n)
		UnknownAndNumberDivMod = UnknownVar | NumberVar | DivisionModulo, // (x /% n)
		UnknownAndNumberPow = UnknownVar | NumberVar | Power, // (x ^ n)

		UnknownAndSubtree = UnknownVar | Subtree, // (x ? A)
		NumberAndSubtree = NumberVar | Subtree, // (n ? A)

		UnknownAndSubtreeMul = UnknownAndSubtree | Multiplication, // (x * A)
		UnknownAndSubtreeAddSub = UnknownAndSubtree | AdditionSubstitution, // (x +- A)
		NumberAndSubtreeMul = NumberAndSubtree | Multiplication, // (n * A)
		NumberAndSubtreeAddSub = NumberAndSubtree | AdditionSubstitution, // (n +- A)

		Rule1 = Subtree * Multiplication * UnknownAndSubtreeMul, // ((x * A) * B) -> (x * (A * B))
		Rule2 = Subtree * Multiplication * NumberAndSubtreeMul, // ((n * A) * B) -> (n * (A * B))
		Rule3 = UnknownAndSubtreeMul * UnknownAndSubtreeMul * AdditionSubstitution, // ((x * A) +- (x * B)) -> (x * (A +- B))
		Rule4 = NumberAndSubtreeMul * NumberAndSubtreeMul * AdditionSubstitution, // ((n * A) +- (n * B)) -> (n * (A +- B))
		Rule5 = Subtree * UnknownAndSubtreeAddSub * AdditionSubstitution, // (A +- (x +- B)) -> (x +- (A +- B))
		Rule6 = Subtree * NumberAndSubtreeAddSub * AdditionSubstitution, // (A +- (n +- B)) -> (n +- (A +- B))
	};

	friend NodeRule operator|(NodeRule a, NodeRule b);
	std::string toString(const NodeRule rule) const;

	NodeRule validateRules(NodeRule rule1, NodeRule rule2, NodeRule rule3) const;

	bool nodeHasChildren(const NodePtr& node) const;

	NodeRule getRuleForSubtree(const NodePtr& node) const;
	NodeRule getRuleForNode(const NodePtr& node) const;

    std::string outputInfix(const NodePtr& ptr, bool withBrackets) const;
    std::string outputPostfix(const NodePtr& ptr) const;
    std::string outputPrefix(const NodePtr& ptr) const;

    void buildTree(const std::vector<ExpressionNode>& expressionString);
    void buildReducedFormTree();
	void buildBalancedTree();
	void distributeSubtrees(const NodePtr& node, OperatorType parentOp, bool isLeft);

	NodePtr reduceNode(const NodePtr& parent);

	std::vector<ExpressionNode> parseExpression(const std::string& expressionString) const;

	// helpers
	NodePtr allocateNode(const ExpressionNode& node) const;
	ExpressionNode getExpressionNode(const NodePtr& ptr) const;
	NodePtr calculateTwoNumbers(const NodePtr& node, const ExpressionNode& leftExp, const ExpressionNode& rightExp) const;
	NodePtr evaluateOperatorAndNumber(NodePtr& node, const ExpressionNode& number, bool leftIsOp) const;
	
	NodePtr finalTryToSimplifySubtree(NodePtr& node) const;
	NodePtr simplifyAddition(NodePtr& node) const;
	NodePtr simplifySubstitution(NodePtr& node) const;
	NodePtr simplifyMultiplication(NodePtr& node) const;
	NodePtr simplifyDivision(NodePtr& node) const;

	NodePtr applyRulesToSubTree(NodePtr& parent) const;
	NodePtr applyRule1ToSubTree(NodePtr& parent) const;
	NodePtr applyRule2ToSubTree(NodePtr& parent) const;
	NodePtr applyRule3ToSubTree(NodePtr& parent) const;
	NodePtr applyRule4ToSubTree(NodePtr& parent) const;
	NodePtr applyRule5ToSubTree(NodePtr& parent) const;
	NodePtr applyRule6ToSubTree(NodePtr& parent) const;

	bool subTreesAreEqual(const NodePtr& n1, const NodePtr& n2) const;
	bool nodeHasUnknownExpr(const NodePtr& ptr) const;
	NodePtr evaluateSubTreeWithUnknowns(const NodePtr& ptr) const;

	int getMaximumPowerOfSubtree(const NodePtr& node) const;

	void insertNodeIntoDegreeSubtreesMap(const NodePtr& ptr, int power, OperatorType type);
	// helpers end

	NodePtr m_balancedTreeRootNode;
    NodePtr m_reducedTreeRootNode;

	std::map<int, std::vector<SubtreeWithOperator>> m_degreeSubtrees;

	// unused stuff
    void insert(const ExpressionNode& key, const bool&) override;
    NodePtr insert(NodePtr& node, const typename AbstractBaseTree::KVPair& keyValue) override;
    bool remove(const ExpressionNode& key) override;
	NodePtr remove(NodePtr &p, const ExpressionNode &key) override;
	NodePtr find(const NodePtr& node, const ExpressionNode& key) const override;
	iterator find(const ExpressionNode& key) const override;
	iterator begin() const override;
};
