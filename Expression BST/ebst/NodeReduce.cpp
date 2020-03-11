#include "EBST.h"

void EBST::buildReducedFormTree(const NodePtr& node) {
	m_reducedTreeRootNode = reduceNode(node);
}

EBST::NodePtr EBST::reduceNode(const NodePtr &parent) const {
	auto newNode = allocateNode(parent->m_keyValue.first);

	auto left = parent->m_left;
	auto right = parent->m_right;
	if (left && right) {
		newNode->m_left = reduceNode(left);
		newNode->m_right = reduceNode(right);

		auto l = outputInfix(newNode->m_left, true);
		auto r = outputInfix(newNode->m_right, true);

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
			const auto& num = leftExprIsOperator ? rightExp : leftExp;
			newNode = simplifyOperatorAndNumber(newNode, num, leftExprIsOperator);
		} else if (nodeHasUnknownExpr(newNode)) {
			return simplifySubTreeWithUnknowns(newNode);
		}

		return simplifySubtree(newNode);
	}

	return newNode;
}
