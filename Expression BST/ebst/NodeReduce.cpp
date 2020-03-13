#include "EBST.h"

EBST::NodePtr EBST::buildReducedFormTree(const NodePtr& node) {
	return reduceNode(node);
}

EBST::NodePtr EBST::reduceNode(const NodePtr &parent) const {
	auto newNode = allocateNode(parent->m_keyValue.first);

	auto left = parent->m_left;
	auto right = parent->m_right;
	if (left && right) {
		newNode->m_left = reduceNode(left);
		newNode->m_right = reduceNode(right);

		newNode = applyRulesToSubTree(newNode);

		auto leftExp = getExpressionNode(newNode->m_left);
		auto rightExp = getExpressionNode(newNode->m_right);

		const auto leftExprIsOperator = isOperator(leftExp);
		const auto leftExprIsUnknownOperand = !leftExprIsOperator && isOperandUnknown(leftExp.operandValue());
		const auto rightExprIsOperator = isOperator(rightExp);
		const auto rightExprIsUnknownOperand = !rightExprIsOperator && isOperandUnknown(rightExp.operandValue());

		const auto onlyNumbers = !leftExprIsOperator && !leftExprIsUnknownOperand
		                         && !rightExprIsOperator && !rightExprIsUnknownOperand;

		const auto numberAndOperator = ((leftExprIsOperator && !rightExprIsOperator) && !rightExprIsUnknownOperand)
		                                || ((!leftExprIsOperator && rightExprIsOperator) && !leftExprIsUnknownOperand);

		if (onlyNumbers) {
			return simplifyTwoNumbers(newNode, leftExp, rightExp);
		} else if (numberAndOperator) {
			const auto& op = leftExprIsOperator ? rightExp : leftExp;
			newNode = simplifyOperatorAndNumber(newNode, op, leftExprIsOperator);
		} else if (nodeHasUnknownExpr(newNode)) {
			return simplifySubTreeWithUnknowns(newNode);
		}

		return simplifySubtree(newNode);
	}

	return newNode;
}
