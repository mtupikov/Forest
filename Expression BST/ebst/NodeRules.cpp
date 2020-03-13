#include "EBST.h"

EBST::NodePtr EBST::applyRulesToTree(NodePtr& parent) const {
	auto newNode = applyRulesToSubTree(parent);

	if (nodeHasChildren(newNode)) {
		newNode->m_left = applyRulesToTree(newNode->m_left);
		newNode->m_right = applyRulesToTree(newNode->m_right);
	}

	return newNode;
}

EBST::NodePtr EBST::applyRulesToSubTree(NodePtr& parent) const {
	const auto rule = getRuleForSubtree(parent);

	switch (rule) {
	case NodeRule::Rule1: return applyRule1ToSubTree(parent);
	case NodeRule::Rule2: return applyRule2ToSubTree(parent);
	case NodeRule::Rule3: return applyRule3ToSubTree(parent);
	case NodeRule::Rule4: return applyRule4ToSubTree(parent);
	case NodeRule::Rule5: return applyRule5ToSubTree(parent);
	case NodeRule::Rule6: return applyRule6ToSubTree(parent);
	default: return parent;
	}
}

EBST::NodePtr EBST::applyRule1ToSubTree(NodePtr& parent) const {
	auto& left = parent->m_left;
	auto& right = parent->m_right;

	// ((x * A) * B) -> (x * (A * B))

	const auto leftIsUnknownAndSubtreeMul = getRuleForSubtree(left) == EBST::NodeRule::UnknownAndSubtreeMul;
	auto& unknownAndSubtreeMulNode = leftIsUnknownAndSubtreeMul ? left : right;
	auto& subtreeBNode = leftIsUnknownAndSubtreeMul ? right : left;

	const auto leftIsUnknown = getRuleForSubtree(unknownAndSubtreeMulNode->m_left) == EBST::NodeRule::UnknownVar;
	auto& unknownNode = leftIsUnknown ? unknownAndSubtreeMulNode->m_left : unknownAndSubtreeMulNode->m_right;
	auto& subtreeANode = leftIsUnknown ? unknownAndSubtreeMulNode->m_right : unknownAndSubtreeMulNode->m_left;

	auto newRightNode = allocateNode(ExpressionNode(OperatorType::Multiplication));
	newRightNode->m_left = subtreeANode;
	newRightNode->m_right = subtreeBNode;

	auto result = allocateNode(ExpressionNode(parent->m_keyValue.first));
	result->m_left = unknownNode;
	result->m_right = newRightNode;

	return result;
}

EBST::NodePtr EBST::applyRule2ToSubTree(NodePtr& parent) const {
	auto& left = parent->m_left;
	auto& right = parent->m_right;

	// ((n * A) * B) -> (n * (A * B))

	const auto leftIsNumberAndSubtreeMul = getRuleForSubtree(left) == EBST::NodeRule::NumberAndSubtreeMul;
	auto& numberAndSubtreeMulNode = leftIsNumberAndSubtreeMul ? left : right;
	auto& subtreeBNode = leftIsNumberAndSubtreeMul ? right : left;

	if (subtreeBNode == left) {
		return parent;
	}

	const auto leftIsNumber = getRuleForSubtree(numberAndSubtreeMulNode->m_left) == EBST::NodeRule::NumberVar;
	auto& numberNode = leftIsNumber ? numberAndSubtreeMulNode->m_left : numberAndSubtreeMulNode->m_right;
	auto& subtreeANode = leftIsNumber ? numberAndSubtreeMulNode->m_right : numberAndSubtreeMulNode->m_left;

	auto newRightNode = allocateNode(ExpressionNode(OperatorType::Multiplication));
	newRightNode->m_left = subtreeANode;
	newRightNode->m_right = subtreeBNode;

	auto result = allocateNode(ExpressionNode(parent->m_keyValue.first));
	result->m_left = numberNode;
	result->m_right = newRightNode;

	return result;
}

EBST::NodePtr EBST::applyRule3ToSubTree(NodePtr& parent) const {
	auto& left = parent->m_left;
	auto& right = parent->m_right;

	// ((x * A) +- (x * B)) -> (x * (A +- B))

	const auto parentOp = parent->m_keyValue.first;

	const auto leftLeftIsUnknown = getRuleForSubtree(left->m_left) == EBST::NodeRule::UnknownVar;
	auto& leftUnknownNode = leftLeftIsUnknown ? left->m_left : left->m_right;
	auto& leftSubtreeNode = leftLeftIsUnknown ? left->m_right : left->m_left;

	const auto rightLeftIsUnknown = getRuleForSubtree(right->m_left) == EBST::NodeRule::UnknownVar;
	auto& rightSubtreeNode = rightLeftIsUnknown ? right->m_right : right->m_left;

	auto newRightNode = allocateNode(parentOp);
	newRightNode->m_left = leftSubtreeNode;
	newRightNode->m_right = rightSubtreeNode;

	auto result = allocateNode(ExpressionNode(OperatorType::Multiplication));
	result->m_left = leftUnknownNode;
	result->m_right = newRightNode;

	return result;
}

EBST::NodePtr EBST::applyRule4ToSubTree(NodePtr& parent) const {
	auto& left = parent->m_left;
	auto& right = parent->m_right;

	// ((n * A) +- (n * B)) -> (n * (A +- B))

	const auto leftLeftIsNumber = getRuleForSubtree(left->m_left) == EBST::NodeRule::NumberVar;
	auto& leftNumberNode = leftLeftIsNumber ? left->m_left : left->m_right;
	auto& leftSubtreeNode = leftLeftIsNumber ? left->m_right : left->m_left;

	const auto rightLeftIsNumber = getRuleForSubtree(right->m_left) == EBST::NodeRule::NumberVar;
	auto& rightNumberNode = rightLeftIsNumber ? right->m_left : right->m_right;
	auto& rightSubtreeNode = rightLeftIsNumber ? right->m_right : right->m_left;

	const auto firstNum = leftNumberNode->m_keyValue.first.operandValue().value;
	const auto secondNum = rightNumberNode->m_keyValue.first.operandValue().value;

	if (firstNum != secondNum) {
		return parent;
	}

	const auto parentOp = parent->m_keyValue.first;

	auto newRightNode = allocateNode(parentOp);
	newRightNode->m_left = leftSubtreeNode;
	newRightNode->m_right = rightSubtreeNode;

	auto result = allocateNode(ExpressionNode(OperatorType::Multiplication));
	result->m_left = leftNumberNode;
	result->m_right = newRightNode;

	return result;
}

EBST::NodePtr EBST::applyRule5ToSubTree(NodePtr& parent) const {
	auto& left = parent->m_left;
	auto& right = parent->m_right;

	// (A +- (x +- B)) -> (x +- (A +- B))

	const auto parentOp = parent->m_keyValue.first;

	const auto leftIsUnknownAndSubtreeAddSub = getRuleForSubtree(left) == EBST::NodeRule::UnknownAndSubtreeAddSub;
	auto& unknownAndSubtreeAddSubNode = leftIsUnknownAndSubtreeAddSub ? left : right;
	auto& subtreeANode = leftIsUnknownAndSubtreeAddSub ? right : left;
	const auto subOp = unknownAndSubtreeAddSubNode->m_keyValue.first;

	if (subtreeANode == right) {
		return parent;
	}

	const auto leftIsUnknown = getRuleForSubtree(unknownAndSubtreeAddSubNode->m_left) == EBST::NodeRule::UnknownVar;
	auto& unknownNode = leftIsUnknown ? unknownAndSubtreeAddSubNode->m_left : unknownAndSubtreeAddSubNode->m_right;
	auto& subtreeBNode = leftIsUnknown ? unknownAndSubtreeAddSubNode->m_right : unknownAndSubtreeAddSubNode->m_left;

	auto newRightNode = allocateNode(subOp);
	newRightNode->m_left = subtreeANode;
	newRightNode->m_right = subtreeBNode;

	const auto isSubs = parentOp.operatorType() == OperatorType::Substitution;

	auto result = allocateNode(ExpressionNode(parentOp));
	result->m_left = isSubs ? newRightNode : unknownNode;
	result->m_right = isSubs ? unknownNode : newRightNode;

	return result;
}

EBST::NodePtr EBST::applyRule6ToSubTree(NodePtr& parent) const {
	auto& left = parent->m_left;
	auto& right = parent->m_right;

	// (A +- (n +- B)) -> (n +- (A +- B))

	const auto parentOp = parent->m_keyValue.first;

	const auto leftIsNumberAndSubtreeAddSub = getRuleForSubtree(left) == EBST::NodeRule::NumberAndSubtreeAddSub;
	auto& numberAndSubtreeAddSubNode = leftIsNumberAndSubtreeAddSub ? left : right;
	auto& subtreeANode = leftIsNumberAndSubtreeAddSub ? right : left;
	const auto subtreeOp = numberAndSubtreeAddSubNode->m_keyValue.first;
	const auto isParentOpSub = parentOp.operatorType() == OperatorType::Substitution;

	if (subtreeANode == right) {
		return parent;
	}

	const auto leftIsNumber = getRuleForSubtree(numberAndSubtreeAddSubNode->m_left) == EBST::NodeRule::NumberVar;
	auto& numberNode = leftIsNumber ? numberAndSubtreeAddSubNode->m_left : numberAndSubtreeAddSubNode->m_right;
	auto& subtreeBNode = leftIsNumber ? numberAndSubtreeAddSubNode->m_right : numberAndSubtreeAddSubNode->m_left;

	auto newRightNode = allocateNode(subtreeOp);
	newRightNode->m_left = subtreeANode;
	newRightNode->m_right = subtreeBNode;

	auto result = allocateNode(parentOp);
	result->m_left = isParentOpSub ? newRightNode : numberNode;
	result->m_right = isParentOpSub ? numberNode : newRightNode;

	return result;
}

EBST::NodeRule EBST::getRuleForSubtree(const NodePtr& node) const {
	if (!nodeHasChildren(node)) {
		return getRuleForNode(node);
	}

	auto& left = node->m_left;
	auto& right = node->m_right;

	const auto nodeRule = getRuleForNode(node);
	auto leftRule = getRuleForSubtree(left);
	auto rightRule = getRuleForSubtree(right);

	const std::vector<NodeRule> transformedToSubtreeRules {
		NodeRule::UnknownAndNumberMul,
		NodeRule::UnknownAndNumberAddSub,
		NodeRule::UnknownAndNumberDivMod,
		NodeRule::UnknownAndNumberPow,
	};

	const auto transform = [&transformedToSubtreeRules](NodeRule& rule) {
		if (std::find(transformedToSubtreeRules.begin(), transformedToSubtreeRules.end(), rule) != transformedToSubtreeRules.end()) {
			rule = NodeRule::Subtree;
		}
	};

	transform(leftRule);
	transform(rightRule);

	const auto resultRule = validateRules(nodeRule, leftRule, rightRule);

	if (resultRule == NodeRule::NoRule) {
		return NodeRule::Subtree;
	}

	return resultRule;
}

EBST::NodeRule EBST::getRuleForNode(const NodePtr& node) const {
	const auto expr = node->m_keyValue.first;

	switch (expr.type()) {
	case ExpressionType::Operand: {
		if (isOperandUnknown(expr.operandValue())) {
			return NodeRule::UnknownVar;
		}

		return NodeRule::NumberVar;
	}
	case ExpressionType::Operator: {
		const auto opType = expr.operatorType();
		if (opType == OperatorType::Multiplication) {
			return NodeRule::Multiplication;
		} else if (opType == OperatorType::Addition || opType == OperatorType::Substitution) {
			return NodeRule::AdditionSubstitution;
		} else if (opType == OperatorType::Division || opType == OperatorType::Modulo) {
			return NodeRule::DivisionModulo;
		} else if (opType == OperatorType::Power) {
			return NodeRule::Power;
		}
		break;
	}
	default: return NodeRule::NoRule;
	}

	return NodeRule::NoRule;
}

EBST::NodeRule EBST::validateRules(NodeRule rule1, NodeRule rule2, NodeRule rule3) const {
	const std::vector<NodeRule> allowedRulesOr {
		NodeRule::Subtree,
		NodeRule::UnknownVar,
		NodeRule::NumberVar,
		NodeRule::UnknownAndSubtree,
		NodeRule::NumberAndSubtree,
		NodeRule::Multiplication,
		NodeRule::AdditionSubstitution,
		NodeRule::AdditionSubstitution,
		NodeRule::DivisionModulo,
		NodeRule::Power,
		NodeRule::UnknownAndNumberMul,
		NodeRule::UnknownAndNumberAddSub,
		NodeRule::UnknownAndNumberDivMod,
		NodeRule::UnknownAndNumberPow,
		NodeRule::UnknownAndSubtreeMul,
		NodeRule::UnknownAndSubtreeAddSub,
		NodeRule::NumberAndSubtreeMul,
		NodeRule::NumberAndSubtreeAddSub
	};

	const std::vector<NodeRule> allowedRulesMul {
		NodeRule::Rule1,
		NodeRule::Rule2,
		NodeRule::Rule3,
		NodeRule::Rule4,
		NodeRule::Rule5,
		NodeRule::Rule6
	};

	const auto ruleMul = static_cast<NodeRule>(rule1 * rule2 * rule3);
	const auto itMul = std::find(allowedRulesMul.cbegin(), allowedRulesMul.cend(), ruleMul);

	if (itMul != allowedRulesMul.cend()) {
		return ruleMul;
	}

	const auto ruleOr = rule1 | rule2 | rule3;
	const auto it = std::find(allowedRulesOr.cbegin(), allowedRulesOr.cend(), ruleOr);

	if (it != allowedRulesOr.cend()) {
		return ruleOr;
	}

	return NodeRule::NoRule;
}
