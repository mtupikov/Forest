#include "EBST.h"

#include <algorithm>

EBST::NodePtr EBST::buildBalancedTree(const NodePtr& node) {
	splitSubtreesByDegree(node);

	const auto nodeIsNumberZero = [this](const NodePtr& node) {
		return getRuleForNode(node) == NumberVar && getExpressionNode(node).operandValue().value == 0.0;
	};

	auto countDeg = 0;
	std::vector<SubtreeWithOperator> rootTreeVec;
	for (const auto& pair : m_degreeSubtrees) {
		auto vec = pair.second;
		std::partition(vec.begin(), vec.end(), [](const SubtreeWithOperator& s) {
			return s.isLeft && s.op == OperatorType::Addition;
		});
		auto degreeTree = buildTreeFromVectorOfNodes(vec, false, countDeg == 0);

		auto degTreeOp = vec.size() == 1 ? vec.front().op : OperatorType::Addition;

		auto reducedNode = reduceNode(degreeTree);

		if (!nodeIsNumberZero(reducedNode)) {
			rootTreeVec.push_back({ reducedNode, degTreeOp, false });
			++countDeg;
		}
	}

	auto balancedTree = buildTreeFromVectorOfNodes(rootTreeVec, false, true);
	m_isBalanced = true;

	// all simplified to zero
	if (!balancedTree) {
		auto zero = allocateNode(ExpressionNode(0.0));
		m_degreeSubtrees[0].push_back({ zero, OperatorType::Addition, false });
		return zero;
	}

	splitSubtreesByDegree(balancedTree);
	auto maxDegree = 0;
	for (const auto& pair : m_degreeSubtrees) {
		if (pair.second.size() != 1) {
			m_isBalanced = false;
		}

		maxDegree = std::max(maxDegree, pair.first);
	}

	m_maxDegree = maxDegree;

	return balancedTree;
}

void EBST::splitSubtreesByDegree(const NodePtr& root) {
	m_degreeSubtrees.clear();
	distributeSubtrees(root, OperatorType::Addition, OperatorType::Addition, false);
}

void EBST::distributeSubtrees(const NodePtr& node, OperatorType parentOp, OperatorType subParentOp, bool isLeft) {
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
			auto resolvedOp = (parentOp == OperatorType::Substitution && !isLeft) ? parentOp : OperatorType::Addition;
			if (subParentOp == OperatorType::Substitution && isLeft) {
				resolvedOp = OperatorType::Substitution;
			}

			if (countUnknownVars(node) > 1) {
				if (countUnknownVars(left) > 1) {
					node->m_left = reduceNode(left);
				}

				if (countUnknownVars(right) > 1) {
					node->m_right = reduceNode(right);
				}

				auto reducedNode = reduceNode(node);
				if (!subTreesAreEqual(node, reducedNode)) {
					distributeSubtrees(reducedNode, parentOp, subParentOp, isLeft);
					return;
				}
			}

			const auto subtreePower = getMaximumPowerOfSubtree(node);
			insertNodeIntoDegreeSubtreesMap(node, subtreePower, resolvedOp, isLeft);
			return;
		}

		const auto nodeOp = node->m_keyValue.first.operatorType();

		distributeSubtrees(left, nodeOp, parentOp, true);
		distributeSubtrees(right, nodeOp, parentOp, false);
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

EBST::NodePtr EBST::buildTreeFromVectorOfNodes(const std::vector<SubtreeWithOperator>& vec, bool hasParentTree, bool isBalancedFirst) const {
	for (auto it = vec.cbegin(); it < vec.cend();) {
		auto& subtree = it->subtree;

		auto next = std::next(it, 1);
		if (next != vec.cend()) {
			auto opNode = allocateNode(ExpressionNode(next->op));

			opNode->m_left = subtree;
			opNode->m_right = buildTreeFromVectorOfNodes(std::vector(next, vec.cend()), true);
			return opNode;
		}

		const auto nodeIsSimple = (nodeHasChildren(subtree)
		                          && !nodeHasChildren(subtree->m_left)
		                          && !nodeHasChildren(subtree->m_right))
		                          || !nodeHasChildren(subtree);

		if (!hasParentTree && it->op == OperatorType::Substitution && nodeIsSimple && isBalancedFirst && !it->isLeft) {
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
