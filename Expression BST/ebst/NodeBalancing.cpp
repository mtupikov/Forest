#include "EBST.h"

void EBST::buildBalancedTree() {
	distributeSubtrees(m_rootNode, OperatorType::Invalid, false);

//	for (const auto& pair : m_degreeSubtrees) {
//		std::cout << "Degree: " << pair.first << std::endl;
//		for (const auto& node : pair.second) {
//			std::cout << ExpressionNode(node.op) << " expr: " << outputInfix(node.subtree, true) << std::endl;
//		}
//	}
}

void EBST::distributeSubtrees(const NodePtr& node, OperatorType parentOp, bool isLeft) {
	auto left = node->m_left;
	auto right = node->m_right;

	auto isAddSubOperator = [](const NodePtr& ptr) {
		const auto expr = ptr->m_keyValue.first;

		if (!isOperator(expr)) {
			return false;
		}

		const auto opType = expr.operatorType();
		return opType == OperatorType::Substitution || opType == OperatorType::Addition;
	};

	if (left && right) {
		const auto nodeIsAddSub = isAddSubOperator(node);

		if (!nodeIsAddSub) {
			const auto resolvedIfLeftOp = isLeft ? OperatorType::Addition : parentOp;

			if (countUnknownVars(node) > 1) {
				if (countUnknownVars(left) > 1) {
					node->m_left = reduceNode(left);
				}

				if (countUnknownVars(right) > 1) {
					node->m_right = reduceNode(right);
				}

				auto reducedNode = reduceNode(node);
				if (!subTreesAreEqual(node, reducedNode)) {
					distributeSubtrees(reducedNode, parentOp, isLeft);
					return;
				}
			}

			const auto subtreePower = getMaximumPowerOfSubtree(node);
			insertNodeIntoDegreeSubtreesMap(node, subtreePower, resolvedIfLeftOp);
			return;
		}

		const auto nodeOp = node->m_keyValue.first.operatorType();

		distributeSubtrees(left, nodeOp, true);
		distributeSubtrees(right, nodeOp, false);
		return;
	}

	if (isOperator(node->m_keyValue.first)) {
		return;
	}

	const auto resolvedIfLeftOp = isLeft ? OperatorType::Addition : parentOp;
	const auto isUnknown = isOperandUnknown(node->m_keyValue.first.operandValue());

	if (!isUnknown && node->m_keyValue.first.operandValue().value == 0.0) {
		return;
	}

	insertNodeIntoDegreeSubtreesMap(node, isUnknown ? 1 : 0, resolvedIfLeftOp);
}

void EBST::insertNodeIntoDegreeSubtreesMap(const NodePtr& node, int power, OperatorType type) {
	if (m_degreeSubtrees.count(power) == 0) {
		m_degreeSubtrees[power] = {};
	}

	auto& degreeVec = m_degreeSubtrees[power];
	degreeVec.push_back({ node, type });
}
