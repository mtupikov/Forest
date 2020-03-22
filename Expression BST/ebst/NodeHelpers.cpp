#include "EBST.h"

EBST::NodePtr EBST::allocateNode(const ExpressionNode& node) const {
	auto pair = std::make_pair(node, false);
	return std::make_shared<EBST::Node>(pair);
}

ExpressionNode EBST::getExpressionNode(const NodePtr& ptr) const {
	return ptr->m_keyValue.first;
}

bool EBST::subTreesAreEqual(const NodePtr& n1, const NodePtr& n2) const {
	if (!n1 && !n2) {
		return true;
	}

	const auto getExpr = [](const NodePtr& ptr) {
		return ptr->m_keyValue.first;
	};

	const auto parent1Expr = getExpr(n1);
	const auto parent2Expr = getExpr(n2);

	return parent1Expr == parent2Expr
	       && subTreesAreEqual(n1->m_left, n2->m_left)
	       && subTreesAreEqual(n1->m_right, n2->m_right);
}

bool EBST::nodeHasUnknownExpr(const NodePtr& ptr) const {
	if (!nodeHasChildren(ptr)) {
		return false;
	}

	auto left = ptr->m_left;
	auto right = ptr->m_right;

	const auto leftExpr = getExpressionNode(left);
	const auto rightExpr = getExpressionNode(right);
	const auto operandsUnknown = isOperandUnknown(leftExpr.operandValue()) || isOperandUnknown(rightExpr.operandValue());
	const auto areOperators = isOperator(leftExpr) || isOperator(rightExpr);

	return operandsUnknown && !areOperators;
}

int EBST::getMaximumPowerOfSubtree(const EBST::NodePtr& node) const {
	if (nodeHasChildren((node))) {
		auto left = node->m_left;
		auto right = node->m_right;

		if (isOperator(node->m_keyValue.first) && node->m_keyValue.first.operatorType() == OperatorType::Power) {
			return static_cast<int>(right->m_keyValue.first.operandValue().value);
		}

		const auto leftPower = getMaximumPowerOfSubtree(left);
		const auto rightPower = getMaximumPowerOfSubtree(right);

		return std::max(leftPower, rightPower);
	}

	const auto nodeIsUnknownVar = isOperandUnknown(node->m_keyValue.first.operandValue());
	return nodeIsUnknownVar ? 1 : 0;
}

int EBST::countUnknownVars(const NodePtr& node) const {
	if (nodeHasChildren(node)) {
		auto left = node->m_left;
		auto right = node->m_right;

		const auto leftCount = countUnknownVars(left);
		const auto rightCount = countUnknownVars(right);

		return leftCount + rightCount;
	}

	const auto nodeIsUnknownVar = isOperandUnknown(node->m_keyValue.first.operandValue());
	return nodeIsUnknownVar ? 1 : 0;
}

bool EBST::nodeHasChildren(const NodePtr& node) const {
	return node->m_left && node->m_right;
}

double EBST::retrieveNumberFromNode(const NodePtr& node, OperatorType prevOp, bool isFirst) const {
	const auto rule = getRuleForNode(node);

	const auto modifier = prevOp == OperatorType::Substitution && !isFirst ? -1.0 : 1.0;

	if (rule == NodeRule::NumberVar) {
		return node->m_keyValue.first.operandValue().value * modifier;
	} else if (rule == NodeRule::Multiplication) {
		return node->m_left->m_keyValue.first.operandValue().value * modifier;
	} else if (rule == NodeRule::DivisionModulo) {
		if (node->m_keyValue.first.operatorType() == OperatorType::Division) {
			return modifier / node->m_right->m_keyValue.first.operandValue().value;
		}

		const auto expr = ExpressionNode(modifier) % node->m_right->m_keyValue.first;
		return expr.operandValue().value;
	}

	return modifier;
}
