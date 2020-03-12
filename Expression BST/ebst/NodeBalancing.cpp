#include "EBST.h"

#include <algorithm>

EBST::NodePtr EBST::buildBalancedTree(const NodePtr& node) {
	splitSubtreesByDegree(node);

	std::vector<SubtreeWithOperator> rootTreeVec;
	for (const auto& pair : m_degreeSubtrees) {
		auto vec = pair.second;
		std::partition(vec.begin(), vec.end(), [](const SubtreeWithOperator& s) {
			return s.isLeft && s.op == OperatorType::Addition;
		});
		auto degreeTree = buildTreeFromVectorOfNodes(vec);

		auto degTreeOp = vec.size() == 1 ? vec.front().op : OperatorType::Addition;
		rootTreeVec.push_back({ reduceNode(degreeTree), degTreeOp, false });
	}

	auto balancedTree = buildTreeFromVectorOfNodes(rootTreeVec);

	m_isBalanced = true;
	splitSubtreesByDegree(balancedTree);
	for (const auto& pair : m_degreeSubtrees) {
		if (pair.second.size() != 1) {
			m_isBalanced = false;
			break;
		}
	}

	return balancedTree;
}

void EBST::splitSubtreesByDegree(const NodePtr& root) {
	m_degreeSubtrees.clear();
	distributeSubtrees(root, OperatorType::Addition, false);
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
			insertNodeIntoDegreeSubtreesMap(node, subtreePower, resolvedIfLeftOp, isLeft);
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

	const auto isUnknown = isOperandUnknown(node->m_keyValue.first.operandValue());

	if (!isUnknown && node->m_keyValue.first.operandValue().value == 0.0) {
		return;
	}

	insertNodeIntoDegreeSubtreesMap(node, isUnknown ? 1 : 0, parentOp, isLeft);
}

void EBST::insertNodeIntoDegreeSubtreesMap(const NodePtr& node, int power, OperatorType type, bool isLeft) {
	if (m_degreeSubtrees.count(power) == 0) {
		m_degreeSubtrees[power] = {};
	}

	auto& degreeVec = m_degreeSubtrees[power];
	degreeVec.push_back({ node, type, isLeft });
}

EBST::NodePtr EBST::buildTreeFromVectorOfNodes(const std::vector<SubtreeWithOperator>& vec, bool hasParentTree) const {
	for (auto it = vec.cbegin(); it < vec.cend();) {
		auto next = std::next(it, 1);
		if (next != vec.cend()) {
			auto opNode = allocateNode(ExpressionNode(next->op));

			opNode->m_left = it->subtree;
			opNode->m_right = buildTreeFromVectorOfNodes(std::vector(next, vec.cend()), true);
			return opNode;
		}

		auto& subtree = it->subtree;
		const auto nodeIsSimple = (nodeHasChildren(subtree)
		                          && !nodeHasChildren(subtree->m_left)
		                          && !nodeHasChildren(subtree->m_right))
		                          || !nodeHasChildren(subtree);

		if (!hasParentTree && it->op == OperatorType::Substitution && nodeIsSimple) {
			auto op = allocateNode(ExpressionNode(OperatorType::Multiplication));
			op->m_left = allocateNode(ExpressionNode(-1.0));
			op->m_right = it->subtree;

			return op;
		}

		return it->subtree;
	}

	return {};
}

bool EBST::treeIsBalanced() const {
	return m_isBalanced;
}
