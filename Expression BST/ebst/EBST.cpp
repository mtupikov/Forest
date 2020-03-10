#include "EBST.h"

#include <cctype>
#include <stack>
#include <exception>
#include <algorithm>

ExpressionTreeException::ExpressionTreeException(const std::string& message, int column)
    : std::logic_error(message)
    , m_message(message)
    , m_column(column)
{
}

std::string ExpressionTreeException::errorMessage() const {
    return m_message;
}

int ExpressionTreeException::column() const {
    return m_column;
}

EBST::EBST(const std::string& expressionString) {
    auto parsedExp = parseExpression(expressionString);
    buildTree(parsedExp);
	buildBalancedTree();
    buildReducedFormTree();
}

std::string EBST::toString(OutputType type) const {
    switch (type) {
    case OutputType::InfixWithParentheses:
    case OutputType::Infix: return outputInfix(m_rootNode, type == OutputType::InfixWithParentheses);
    case OutputType::Postfix: return outputPostfix(m_rootNode);
    case OutputType::Prefix: return outputPrefix(m_rootNode);
    case OutputType::ReducedInfixWithParentheses:
    case OutputType::ReducedInfix: return outputInfix(m_reducedTreeRootNode, type == OutputType::ReducedInfixWithParentheses);
    }

    return {};
}

std::vector<ExpressionResult> EBST::calculateResult() const {
	return std::vector<ExpressionResult>();
}

bool EBST::nodeHasChildren(const NodePtr& node) const {
	return node->m_left && node->m_right;
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

void EBST::buildTree(const std::vector<ExpressionNode>& expr) {
    std::stack<EBST::NodePtr> stack;
    EBST::NodePtr t;

    for (const auto& node : expr) {
        auto pair = std::make_pair(node, false);
        t = std::make_shared<EBST::Node>(pair);
        if (node.type() == ExpressionType::Operator) {
            auto t1 = stack.top();
            stack.pop();
            auto t2 = stack.top();
            stack.pop();

            t->m_left = t2;
            t->m_right = t1;
        }

        stack.push(t);
    }

    m_rootNode = stack.top();
    stack.pop();
}

void EBST::buildReducedFormTree() {
	m_reducedTreeRootNode = reduceNode(m_rootNode);
}

void EBST::buildBalancedTree() {
	distributeSubtrees(m_rootNode, OperatorType::Invalid, false);

	for (const auto& pair : m_degreeSubtrees) {
		std::cout << "Degree: " << pair.first << std::endl;
		for (const auto& node : pair.second) {
			std::cout << ExpressionNode(node.op) << " expr: " << outputInfix(node.subtree, true) << std::endl;
		}
	}
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
	insertNodeIntoDegreeSubtreesMap(node, isUnknown ? 1 : 0, resolvedIfLeftOp);
}

EBST::NodePtr EBST::reduceNode(const NodePtr &parent) {
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

		const auto ok1 = outputInfix(newNode, true);

		if (onlyNumbers) {
			return calculateTwoNumbers(newNode, leftExp, rightExp);
		} else if (numberAndOperator) {
			const auto& num = leftExprIsOperator ? rightExp : leftExp;
			newNode = evaluateOperatorAndNumber(newNode, num, leftExprIsOperator);
		} else if (nodeHasUnknownExpr(newNode)) {
			return evaluateSubTreeWithUnknowns(newNode);
        }

		return finalTryToSimplifySubtree(newNode);
	}

	return newNode;
}

std::vector<ExpressionNode> EBST::parseExpression(const std::string& expr) const {
    std::vector<ExpressionNode> output;
    std::stack<ExpressionNode> stack;

    const auto readToken = [&expr](auto& it) -> std::optional<ExpressionNode> {
        auto dotFound = false;

        if (std::isdigit(*it) || *it == '-' || *it == '+') {
            auto begin = it;
            ++it;
            while (it != expr.cend() && (std::isdigit(*it) || (*it == '.' && !dotFound))) {
                if (*it == '.') {
                    dotFound = true;
                }

                ++it;
            }

            const auto strNum = std::string(begin, it);
            --it;

            return parseOperandNodeFromString(strNum);
        } else if (std::isalpha(*it)) {
            auto begin = it;
            ++it;
            while (it != expr.cend() && (std::isalpha(*it))) {
                ++it;
            }

            const auto strVal = std::string(begin, it);
            --it;

            if (strVal.length() > 1) {
				throw ExpressionTreeException("Unknown operand must be 1 character", static_cast<int>(std::distance(expr.cbegin(), it)));
            }

            return parseOperandNodeFromString(strVal);
        }

		throw ExpressionTreeException("Invalid token", static_cast<int>(std::distance(expr.cbegin(), it)));
        return std::nullopt;
    };

    const auto nextTokenIsNumber = [&expr](auto it) {
        const auto next = std::next(it, 1);
        return next != expr.cend() && std::isdigit(*next);
    };

	const auto expressionNodeIsOperator = [](const std::optional<ExpressionNode>& op) {
        return op.has_value() && op.value().type() == ExpressionType::Operator;
    };

	const auto distanceFromBegin = [&expr](const auto& it) {
		return static_cast<int>(std::distance(expr.cbegin(), it));
	};

    std::optional<ExpressionNode> lastExpressionNode;
    for (auto it = expr.cbegin(); it < expr.cend(); ++it) {
        const auto c = *it;

        if (std::isspace(c)) {
            continue;
        }

        auto pOp = parseOperatorNodeFromChar(c);

		const auto nextTokenIsNumberAndPrevIsOperator = nextTokenIsNumber(it)
		                                                && expressionNodeIsOperator(lastExpressionNode);

        if (pOp.has_value() && !isBracket(pOp.value())
		    && !nextTokenIsNumberAndPrevIsOperator) {

			if (expressionNodeIsOperator(lastExpressionNode) && !isBracket(lastExpressionNode.value())) {
				throw ExpressionTreeException("Operator can't be placed after another operator", distanceFromBegin(it));
			}

			auto op = pOp.value();

            if (!stack.empty()) {
                auto top = stack.top();

                while (!isBracket(top) && (op <= top)) {
                    stack.pop();
                    output.push_back(top);

                    if (!stack.empty()) {
                        top = stack.top();
                    } else {
                        break;
                    }
                }
            }

            stack.push(op);
        } else if (c == '(') {
            if (pOp.has_value()) {
                stack.push(pOp.value());
            } else {
				throw ExpressionTreeException("Left bracket is invalid", distanceFromBegin(it));
            }
        } else if (c == ')') {
            auto top = stack.top();
            while (stack.top().operatorType() != OperatorType::BracketLeft) {
                output.push_back(top);
                stack.pop();
                if (stack.empty()) {
					throw ExpressionTreeException("Missing '(' parentheses" , distanceFromBegin(it));
                }
                top = stack.top();
            }

            if (!stack.empty() && stack.top().operatorType() == OperatorType::BracketLeft) {
                stack.pop();
            }
        } else {
            if (lastExpressionNode.has_value() && lastExpressionNode.value().type() == ExpressionType::Operand) {
				throw ExpressionTreeException("Missing operator between operands" , distanceFromBegin(it));
            }

            pOp = readToken(it);
            if (pOp.has_value()) {
                output.push_back(pOp.value());
            } else {
				throw ExpressionTreeException("Invalid operand" , distanceFromBegin(it));
            }
        }

        lastExpressionNode = pOp;
    }

    while (!stack.empty()) {
        auto rToken = stack.top();
        if (isBracket(rToken)) {
			throw ExpressionTreeException("Missing ')' parentheses" , distanceFromBegin(expr.cend()));
        }
        output.push_back(rToken);
        stack.pop();
    }

	return output;
}

bool EBST::nodeHasUnknownExpr(const NodePtr& ptr) const {
	auto left = ptr->m_left;
	auto right = ptr->m_right;

	if (!nodeHasChildren(ptr)) {
		return false;
	}

	const auto leftExpr = getExpressionNode(left);
	const auto rightExpr = getExpressionNode(right);
	const auto operandsUnknown = isOperandUnknown(leftExpr.operandValue()) || isOperandUnknown(rightExpr.operandValue());
	const auto areOperators = isOperator(leftExpr) || isOperator(rightExpr);
	
	return operandsUnknown && !areOperators;
}

EBST::NodePtr EBST::evaluateSubTreeWithUnknowns(const NodePtr& ptr) const {
	auto& left = ptr->m_left;
	auto& right = ptr->m_right;

	const auto parentExpr = getExpressionNode(ptr);
	const auto leftExpr = getExpressionNode(left);
	const auto rightExpr = getExpressionNode(right);
	const auto leftIsUnknown = isOperandUnknown(leftExpr.operandValue());
	const auto rightIsUnknown = isOperandUnknown(rightExpr.operandValue());
	const auto leftAndRightUnknown = leftIsUnknown && rightIsUnknown;
	const auto leftUnknownRightZero = leftIsUnknown && rightExpr.operandValue().value == 0.0;
	const auto rightUnknownLeftZero = rightIsUnknown && leftExpr.operandValue().value == 0.0;
	const auto leftUnknownRightOne = leftIsUnknown && rightExpr.operandValue().value == 1.0;
	const auto rightUnknownLeftOne = rightIsUnknown && leftExpr.operandValue().value == 1.0;

	const auto resetLeftRight = [&left, &right] {
		left.reset();
		right.reset();
	};

	switch (parentExpr.operatorType()) {
	case OperatorType::Substitution: {
		if (leftAndRightUnknown) {
			resetLeftRight();
			ptr->m_keyValue.first = ExpressionNode(0.0);
		} else if (leftUnknownRightZero) {
			resetLeftRight();
			ptr->m_keyValue.first = ExpressionNode(leftExpr);
		} else if (rightUnknownLeftZero) {
			ptr->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);
			left->m_keyValue.first = ExpressionNode(-1.0);
			right->m_keyValue.first = rightExpr;
		}
		break;
	}
	case OperatorType::Addition: {
		if (leftAndRightUnknown) {
			ptr->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);
			right->m_keyValue.first = ExpressionNode(2.0);
		} else if (leftUnknownRightZero || rightUnknownLeftZero) {
			resetLeftRight();
			ptr->m_keyValue.first = ExpressionNode(leftIsUnknown ? leftExpr : rightExpr);
		}
		break;
	}
	case OperatorType::Multiplication: {
		if (leftAndRightUnknown) {
			ptr->m_keyValue.first = ExpressionNode(OperatorType::Power);
			right->m_keyValue.first = ExpressionNode(2.0);
		} else if (leftUnknownRightZero || rightUnknownLeftZero) {
			resetLeftRight();
			ptr->m_keyValue.first = ExpressionNode(0.0);
		} else if (leftUnknownRightOne) {
			ptr->m_keyValue.first = leftExpr;
			resetLeftRight();
		} else if (rightUnknownLeftOne) {
			ptr->m_keyValue.first = rightExpr;
			resetLeftRight();
		}
		break;
	}
	case OperatorType::Division: {
		if (leftAndRightUnknown) {
			resetLeftRight();
			ptr->m_keyValue.first = ExpressionNode(1.0);
		} else if (leftUnknownRightZero) {
			throw ExpressionTreeException("Division by zero", 0);
		} else if (rightUnknownLeftZero) {
			resetLeftRight();
			ptr->m_keyValue.first = ExpressionNode(0.0);
		} else if (leftUnknownRightOne) {
			ptr->m_keyValue.first = leftExpr;
			resetLeftRight();
		}
		break;
	}
	case OperatorType::Modulo: {
		if (leftAndRightUnknown) {
			resetLeftRight();
			ptr->m_keyValue.first = ExpressionNode(1.0);
		} else if (leftUnknownRightZero) {
			throw ExpressionTreeException("Division by zero", 0);
		} else if (rightUnknownLeftZero) {
			resetLeftRight();
			ptr->m_keyValue.first = ExpressionNode(0.0);
		} else if (leftUnknownRightOne) {
			ptr->m_keyValue.first = leftExpr;
			resetLeftRight();
		}
		break;
	}
	case OperatorType::Power: {
		if (leftAndRightUnknown) {
			break;
		} else if (leftUnknownRightZero) {
			resetLeftRight();
			ptr->m_keyValue.first = ExpressionNode(1.0);
		} else if (rightUnknownLeftZero) {
			resetLeftRight();
			ptr->m_keyValue.first = ExpressionNode(0.0);
		} else if (leftUnknownRightOne) {
			ptr->m_keyValue.first = leftExpr;
			resetLeftRight();
		} else if (rightUnknownLeftOne) {
			resetLeftRight();
			ptr->m_keyValue.first = ExpressionNode(1.0);
		}
		break;
	}
	default: assert(false && "invalid operator");
	}

	return ptr;
}

int EBST::getMaximumPowerOfSubtree(const EBST::NodePtr& node) const {
	auto left = node->m_left;
	auto right = node->m_right;

	if (left && right) {
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

void EBST::insertNodeIntoDegreeSubtreesMap(const NodePtr& node, int power, OperatorType type) {
	if (m_degreeSubtrees.count(power) == 0) {
		m_degreeSubtrees[power] = {};
	}

	auto& degreeVec = m_degreeSubtrees[power];
	degreeVec.push_back({ node, type });
}

EBST::NodePtr EBST::allocateNode(const ExpressionNode& node) const {
	auto pair = std::make_pair(node, false);
	return std::make_shared<EBST::Node>(pair);
}

ExpressionNode EBST::getExpressionNode(const NodePtr& ptr) const {
	return ptr->m_keyValue.first;
}

EBST::NodePtr EBST::calculateTwoNumbers(const NodePtr& node, const ExpressionNode& leftExp, const ExpressionNode& rightExp) const {
	const auto expressionOperator = getExpressionNode(node);

	switch (expressionOperator.operatorType()) {
	case OperatorType::Substitution: return allocateNode(leftExp - rightExp);
	case OperatorType::Addition: return allocateNode(leftExp + rightExp);
	case OperatorType::Multiplication: return allocateNode(leftExp * rightExp);
	case OperatorType::Division: return allocateNode(leftExp / rightExp);
	case OperatorType::Modulo: return allocateNode(leftExp % rightExp);
	case OperatorType::Power: return allocateNode(leftExp ^ rightExp);
	default: assert(false && "invalid operator");
	}

	return NodePtr();
}

EBST::NodePtr EBST::evaluateOperatorAndNumber(NodePtr& node, const ExpressionNode& number, bool leftIsOp) const {
	const auto expressionOperator = getExpressionNode(node);
	const auto numberIsZero = number.operandValue().value == 0.0;
	const auto numberIsOne = number.operandValue().value == 1.0;

	if (!numberIsZero && !numberIsOne) {
		return node;
	}

	auto& left = node->m_left;
	auto& right = node->m_right;

	const auto resetLeftRight = [&left, &right] {
		left.reset();
		right.reset();
	};

	switch (expressionOperator.operatorType()) {
	case OperatorType::Addition:
	case OperatorType::Substitution: {
		if (numberIsZero) {
			node = leftIsOp ? node->m_left : node->m_right;
		}
		return node;
	}
	case OperatorType::Multiplication: {
		if (numberIsZero) {
			resetLeftRight();
			node->m_keyValue.first = ExpressionNode(0.0);
		} else {
			node = leftIsOp ? left : right;
		}
		return node;
	}
	case OperatorType::Division:
	case OperatorType::Modulo: {
		if (numberIsZero) {
			if (leftIsOp) {
				throw ExpressionTreeException("Division by zero", 0);
			} else {
				resetLeftRight();
				node->m_keyValue.first = ExpressionNode(0.0);
			}
		} else if (leftIsOp) {
			node = leftIsOp ? left : right;
		}
		return node;
	}
	case OperatorType::Power: {
		if (numberIsZero) {
			if (leftIsOp) {
				resetLeftRight();
				node->m_keyValue.first = ExpressionNode(1.0);
			} else {
				resetLeftRight();
				node->m_keyValue.first = ExpressionNode(0.0);
			}
		} else {
			if (leftIsOp) {
				node = leftIsOp ? left : right;
			} else {
				resetLeftRight();
				node->m_keyValue.first = ExpressionNode(1.0);
			}
		}
		return node;
	}
	default: assert(false && "invalid operator");
	}

	return NodePtr();
}

EBST::NodePtr EBST::finalTryToSimplifySubtree(NodePtr& node) const {
	const auto nodeOp = node->m_keyValue.first.operatorType();

	switch (nodeOp) {
	case OperatorType::Addition: return simplifyAddition(node);
	case OperatorType::Substitution: return simplifySubstitution(node);
	case OperatorType::Multiplication: return simplifyMultiplication(node);
	case OperatorType::Modulo:
	case OperatorType::Division: return simplifyDivision(node);
	default: return node;
	}
}

EBST::NodePtr EBST::simplifyAddition(NodePtr& node) const {
	auto& left = node->m_left;
	auto& right = node->m_right;

	const auto leftRule = getRuleForSubtree(left);
	const auto rightRule = getRuleForSubtree(right);
	const auto areEqual = leftRule == rightRule;

	const auto isSimpleRule = [](NodeRule rule) {
		return rule == NodeRule::UnknownVar || rule == NodeRule::NumberVar;
	};

	const auto resetLeftRight = [&left, &right] {
		left.reset();
		right.reset();
	};

	const auto isOpType = [](const NodePtr& node, OperatorType type) {
		return node->m_keyValue.first.operatorType() == type;
	};

	if (areEqual) {
		const auto leftLeftIsUnknown = getRuleForSubtree(left->m_left) == EBST::NodeRule::UnknownVar;
		auto& leftUnknownNode = leftLeftIsUnknown ? left->m_left : left->m_right;
		auto& leftNumberNode = leftLeftIsUnknown ? left->m_right : left->m_left;
		auto& leftNumberOperand = leftNumberNode->m_keyValue.first;

		const auto rightLeftIsUnknown = getRuleForSubtree(right->m_left) == EBST::NodeRule::UnknownVar;
		auto& rightNumberNode = rightLeftIsUnknown ? right->m_right : right->m_left;
        auto& rightUnknownNode = rightLeftIsUnknown ? right->m_left : right->m_right;
        auto& rightNumberOperand = rightNumberNode->m_keyValue.first;

		switch (leftRule) {
		case UnknownAndNumberMul: {
			auto newNumberNode = allocateNode(leftNumberOperand + rightNumberOperand);
			node->m_left = leftUnknownNode;
			node->m_right = newNumberNode;
			node->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);
			
			return node;
		}
		case UnknownAndNumberAddSub: {
			auto leftNumber = leftNumberOperand.operandValue().value;
			if (isOpType(left, OperatorType::Substitution) && leftLeftIsUnknown) {
				leftNumber *= -1;
			}

			auto rightNumber = rightNumberOperand.operandValue().value;
			if (isOpType(right, OperatorType::Substitution) && rightLeftIsUnknown) {
				rightNumber *= -1;
			}

			const auto resNumber = leftNumber + rightNumber;
			const auto resExpr = ExpressionNode(resNumber);

			if (isOpType(left, OperatorType::Substitution) && !leftLeftIsUnknown && rightLeftIsUnknown) {
				node->m_keyValue.first = resExpr;
				resetLeftRight();
				return node;
			} else if (leftLeftIsUnknown && rightLeftIsUnknown) {
				auto newLeftNode = allocateNode(ExpressionNode(OperatorType::Multiplication));
				newLeftNode->m_left = allocateNode(ExpressionNode(2.0));
				newLeftNode->m_right = leftUnknownNode;

				node->m_left = newLeftNode;
				node->m_right = allocateNode(resExpr);

				return node;
			} else if (isOpType(left, OperatorType::Substitution) && !leftLeftIsUnknown
						&& isOpType(right, OperatorType::Substitution) && !rightLeftIsUnknown) {
				auto newLeftNode = allocateNode(ExpressionNode(OperatorType::Multiplication));
				newLeftNode->m_left = allocateNode(ExpressionNode(-2.0));
				newLeftNode->m_right = leftUnknownNode;

				node->m_left = newLeftNode;
				node->m_right = allocateNode(resExpr);

				return node;
			}

			break;
		}
		case UnknownAndNumberPow: {
			const auto leftNumber = leftNumberOperand.operandValue().value;
			const auto rightNumber = rightNumberOperand.operandValue().value;

			if (leftNumber == rightNumber && leftLeftIsUnknown && rightLeftIsUnknown) {
				node->m_left = allocateNode(ExpressionNode(2.0));
				node->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);
			}

			return node;
		}
		case NumberAndSubtreeMul: {
            if (subTreesAreEqual(leftUnknownNode, rightUnknownNode)) {
                const auto resExpr = getExpressionNode(leftNumberNode) + getExpressionNode(rightNumberNode);
                auto newNumNode = allocateNode(resExpr);
                node->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);
                node->m_left = leftUnknownNode;
                node->m_right = newNumNode;
            }
		    return node;
		}
        default: return node;
		}
	} else if (isSimpleRule(leftRule) || isSimpleRule(rightRule)) {
		const auto leftIsSimple = isSimpleRule(leftRule);
		const auto simpleRule = leftIsSimple ? leftRule : rightRule;
		const auto simpleRuleIsNumber = simpleRule == NodeRule::NumberVar;
		const auto complexRule = leftIsSimple ? rightRule : leftRule;

		const auto simpleNode = leftIsSimple ? left : right;
		const auto simpleNodeNum = simpleNode->m_keyValue.first.operandValue().value;
		const auto complexNode = leftIsSimple ? right : left;
		const auto complexLeftIsUnknown = getRuleForNode(complexNode->m_left) == NodeRule::UnknownVar;
		const auto complexNumNode = complexLeftIsUnknown ? complexNode->m_right : complexNode->m_left;
		const auto complexUnkNode = complexLeftIsUnknown ? complexNode->m_left : complexNode->m_right;
		const auto complexNodeNum = complexNumNode->m_keyValue.first.operandValue().value;
		const auto complexOpIsAdd = complexNode->m_keyValue.first.operatorType() == OperatorType::Addition;

		switch (complexRule) {
		case UnknownAndNumberAddSub: {
			if (simpleRuleIsNumber) {
				double resNum;
				if (!complexOpIsAdd && complexLeftIsUnknown) {
					resNum = simpleNodeNum - complexNodeNum;
				} else {
					resNum = complexNodeNum + simpleNodeNum;
				}

				if (resNum == 0.0) {
					node = complexUnkNode;
					return node;
				}

				const auto newNumNode = allocateNode(ExpressionNode(resNum));
				node->m_left = complexUnkNode;
				node->m_right = newNumNode;

				return node;
			} else {
				if (!complexOpIsAdd && !complexLeftIsUnknown) {
					node->m_keyValue.first = ExpressionNode(complexNodeNum);
					resetLeftRight();

					return node;
				} else {
					auto newNode = allocateNode(ExpressionNode(OperatorType::Multiplication));
					newNode->m_left = allocateNode(ExpressionNode(2.0));
					newNode->m_right = complexUnkNode;

					node->m_left = newNode;
					node->m_right = complexNumNode;

					return node;
				}
			}
		}
		case UnknownAndNumberMul: {
			if (!simpleRuleIsNumber) {
				const auto newNum = complexNodeNum + 1;
				node->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);
				node->m_left = complexUnkNode;
				node->m_right = allocateNode(ExpressionNode(newNum));

				return node;
			}
		}
		case UnknownAndNumberDivMod:
		case UnknownAndNumberPow: {
			if (simpleNodeNum == 0.0) {
				node = complexNode;

				return node;
			}
			break;
		}
		default: return node;
		}
	}

	return node;
}

EBST::NodePtr EBST::simplifySubstitution(NodePtr& node) const {
	auto& left = node->m_left;
	auto& right = node->m_right;

	const auto leftRule = getRuleForSubtree(left);
	const auto rightRule = getRuleForSubtree(right);
	const auto areEqual = leftRule == rightRule;

	const auto isSimpleRule = [](NodeRule rule) {
		return rule == NodeRule::UnknownVar || rule == NodeRule::NumberVar;
	};

	const auto resetLeftRight = [&left, &right] {
		left.reset();
		right.reset();
	};

	const auto isOpType = [](const NodePtr& node, OperatorType type) {
		return node->m_keyValue.first.operatorType() == type;
	};

	if (areEqual) {
		const auto leftLeftIsUnknown = getRuleForSubtree(left->m_left) == EBST::NodeRule::UnknownVar;
		auto& leftUnknownNode = leftLeftIsUnknown ? left->m_left : left->m_right;
		auto& leftNumberNode = leftLeftIsUnknown ? left->m_right : left->m_left;
		auto& leftNumberOperand = leftNumberNode->m_keyValue.first;

		const auto rightLeftIsUnknown = getRuleForSubtree(right->m_left) == EBST::NodeRule::UnknownVar;
		auto& rightNumberNode = rightLeftIsUnknown ? right->m_right : right->m_left;
		auto& rightNumberOperand = rightNumberNode->m_keyValue.first;
        auto& rightUnknownNode = rightLeftIsUnknown ? right->m_left : right->m_right;

		switch (leftRule) {
		case UnknownAndNumberMul: {
			auto newNumberNode = allocateNode(leftNumberOperand - rightNumberOperand);
			node->m_left = leftUnknownNode;
			node->m_right = newNumberNode;
			node->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);
			
			return node;
		}
		case UnknownAndNumberAddSub: {
			auto leftNumber = leftNumberOperand.operandValue().value;
			if (isOpType(left, OperatorType::Substitution) && leftLeftIsUnknown) {
				leftNumber *= -1;
			}

			auto rightNumber = rightNumberOperand.operandValue().value;
			if (isOpType(right, OperatorType::Substitution) && rightLeftIsUnknown) {
				rightNumber *= -1;
			}

			const auto resNumber = leftNumber - rightNumber;
			const auto resExpr = ExpressionNode(resNumber);

			if ((isOpType(left, OperatorType::Addition) && !leftLeftIsUnknown && rightLeftIsUnknown)
				|| (leftLeftIsUnknown && rightLeftIsUnknown)) {
				node->m_keyValue.first = resExpr;
				resetLeftRight();
				return node;
			} else if (leftLeftIsUnknown && isOpType(right, OperatorType::Substitution) && !rightLeftIsUnknown) {
				auto newLeftNode = allocateNode(ExpressionNode(OperatorType::Multiplication));
				newLeftNode->m_left = allocateNode(ExpressionNode(2.0));
				newLeftNode->m_right = leftUnknownNode;

				node->m_left = newLeftNode;
				node->m_right = allocateNode(resExpr);

				return node;
			} else if (isOpType(left, OperatorType::Substitution) && !leftLeftIsUnknown && rightLeftIsUnknown) {
				auto newLeftNode = allocateNode(ExpressionNode(OperatorType::Multiplication));
				newLeftNode->m_left = allocateNode(ExpressionNode(-2.0));
				newLeftNode->m_right = leftUnknownNode;

				node->m_left = newLeftNode;
				node->m_right = allocateNode(resExpr);

				return node;
			}

			break;
		}
		case UnknownAndNumberPow: {
			const auto leftNumber = leftNumberOperand.operandValue().value;
			const auto rightNumber = rightNumberOperand.operandValue().value;

			if (leftNumber == rightNumber && leftLeftIsUnknown && rightLeftIsUnknown) {
				node->m_keyValue.first = ExpressionNode(0.0);
				resetLeftRight();
			}

			return node;
		}
        case NumberAndSubtreeMul: {
            if (subTreesAreEqual(leftUnknownNode, rightUnknownNode)) {
                const auto resExpr = getExpressionNode(leftNumberNode) - getExpressionNode(rightNumberNode);
                auto newNumNode = allocateNode(resExpr);
                node->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);
                node->m_left = leftUnknownNode;
                node->m_right = newNumNode;
            }
            return node;
        }
		default: return node;
		}
	} else if (isSimpleRule(leftRule) || isSimpleRule(rightRule)) {
		const auto leftIsSimple = isSimpleRule(leftRule);
		const auto simpleRule = leftIsSimple ? leftRule : rightRule;
		const auto simpleRuleIsNumber = simpleRule == NodeRule::NumberVar;
		const auto complexRule = leftIsSimple ? rightRule : leftRule;

		const auto simpleNode = leftIsSimple ? left : right;
		const auto simpleNodeNum = simpleNode->m_keyValue.first.operandValue().value;
		const auto complexNode = leftIsSimple ? right : left;
		const auto complexLeftIsUnknown = getRuleForNode(complexNode->m_left) == NodeRule::UnknownVar;
		const auto complexNumNode = complexLeftIsUnknown ? complexNode->m_right : complexNode->m_left;
		const auto complexUnkNode = complexLeftIsUnknown ? complexNode->m_left : complexNode->m_right;
		const auto complexNodeNum = complexNumNode->m_keyValue.first.operandValue().value;
		const auto complexOpIsAdd = complexNode->m_keyValue.first.operatorType() == OperatorType::Addition;

		switch (complexRule) {
		case UnknownAndNumberAddSub: {
			if (simpleRuleIsNumber) {
				double resNum;
				if (!complexOpIsAdd) {
					resNum = -1 * complexNodeNum - simpleNodeNum;
				} else {
					resNum = complexNodeNum - simpleNodeNum;
				}

				if (resNum == 0.0) {
					node = complexUnkNode;
					return node;
				}

				const auto newNumNode = allocateNode(ExpressionNode(resNum));
				node->m_left = complexUnkNode;
				node->m_right = newNumNode;

				return node;
			}

			if (complexOpIsAdd && !complexLeftIsUnknown) {
				node->m_keyValue.first = ExpressionNode(complexNodeNum);
				resetLeftRight();

				return node;
			} else {
				auto newNode = allocateNode(ExpressionNode(OperatorType::Multiplication));
				newNode->m_left = allocateNode(ExpressionNode(-2.0));
				newNode->m_right = complexUnkNode;

				node->m_left = newNode;
				node->m_right = complexNumNode;

				return node;
			}
		}
		case UnknownAndNumberMul: {
			if (!simpleRuleIsNumber) {
				const auto newNum = complexNodeNum - 1;
				node->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);
				node->m_left = complexUnkNode;
				node->m_right = allocateNode(ExpressionNode(newNum));

				return node;
			}
			break;
		}
		case UnknownAndNumberDivMod:
		case UnknownAndNumberPow: {
			if (simpleNodeNum == 0.0) {
				node = complexNode;

				return node;
			}
			break;
		}
		default: return node;
		}
	}

	return node;
}

EBST::NodePtr EBST::simplifyMultiplication(NodePtr& node) const {
	auto& left = node->m_left;
	auto& right = node->m_right;

	const auto leftRule = getRuleForSubtree(left);
	const auto rightRule = getRuleForSubtree(right);

	const auto isSimpleRule = [](NodeRule rule) {
		return rule == NodeRule::UnknownVar || rule == NodeRule::NumberVar;
	};

	if (isSimpleRule(leftRule) || isSimpleRule(rightRule)) {
		const auto leftIsSimple = isSimpleRule(leftRule);
		const auto simpleRule = leftIsSimple ? leftRule : rightRule;
		const auto simpleRuleIsNumber = simpleRule == NodeRule::NumberVar;
		const auto complexRule = leftIsSimple ? rightRule : leftRule;

		const auto simpleNode = leftIsSimple ? left : right;
		const auto simpleNodeNum = simpleNode->m_keyValue.first.operandValue().value;
		const auto complexNode = leftIsSimple ? right : left;
		const auto complexLeftIsUnknown = getRuleForNode(complexNode->m_left) == NodeRule::UnknownVar;
		const auto complexNumNode = complexLeftIsUnknown ? complexNode->m_right : complexNode->m_left;
		const auto complexUnkNode = complexLeftIsUnknown ? complexNode->m_left : complexNode->m_right;
		const auto complexNodeNum = complexNumNode->m_keyValue.first.operandValue().value;

		if (simpleRuleIsNumber && simpleNodeNum == 0.0) {
			return allocateNode(ExpressionNode(0.0));
		}

		switch (complexRule) {
		case UnknownAndNumberAddSub: {
			if (simpleRuleIsNumber) {
				node = complexNode;

				auto newUnknNode = allocateNode(ExpressionNode(OperatorType::Multiplication));
				newUnknNode->m_left = complexUnkNode;
				newUnknNode->m_right = allocateNode(ExpressionNode(simpleNodeNum));

				auto newNumNode = allocateNode(ExpressionNode(simpleNodeNum * complexNodeNum));

				node->m_left = newUnknNode;
				node->m_right = newNumNode;

				return node;
			}

			node = complexNode;

			auto newUnknPowNode = allocateNode(ExpressionNode(OperatorType::Power));
			newUnknPowNode->m_left = complexUnkNode;
			newUnknPowNode->m_right = allocateNode(ExpressionNode(2.0));

			auto newUnknNode = allocateNode(ExpressionNode(OperatorType::Multiplication));
			newUnknNode->m_left = allocateNode(ExpressionNode(complexUnkNode->m_keyValue.first.operandValue().variableName));
			newUnknNode->m_right = allocateNode(ExpressionNode(simpleNodeNum));

			node->m_left = newUnknPowNode;
			node->m_right = newUnknNode;

			return node;
		}
		case UnknownAndNumberMul: {
			if (simpleRuleIsNumber) {
				node = complexNode;
				complexNumNode->m_keyValue.first = ExpressionNode(simpleNodeNum * complexNodeNum);

				return node;
			}

			node = complexNode;

			auto newUnknPowNode = allocateNode(ExpressionNode(OperatorType::Power));
			newUnknPowNode->m_left = complexUnkNode;
			newUnknPowNode->m_right = allocateNode(ExpressionNode(2.0));

			node->m_left = newUnknPowNode;
			node->m_right = complexNumNode;

			return node;
		}
		case UnknownAndNumberPow: {
			if (!simpleRuleIsNumber) {
				node = complexNode;
				complexNumNode->m_keyValue.first = ExpressionNode(complexNodeNum + 1);

				return node;
			}
			break;
		}
		default: return node;
		}
	}

	return node;
}

EBST::NodePtr EBST::simplifyDivision(NodePtr& node) const {
	auto& left = node->m_left;
	auto& right = node->m_right;

	const auto leftRule = getRuleForSubtree(left);
	const auto rightRule = getRuleForSubtree(right);

	const auto isSimpleRule = [](NodeRule rule) {
		return rule == NodeRule::UnknownVar || rule == NodeRule::NumberVar;
	};

	if (isSimpleRule(leftRule) || isSimpleRule(rightRule)) {
		const auto leftIsSimple = isSimpleRule(leftRule);
		const auto simpleRule = leftIsSimple ? leftRule : rightRule;
		const auto simpleRuleIsNumber = simpleRule == NodeRule::NumberVar;

		const auto simpleNode = leftIsSimple ? left : right;
		const auto simpleNodeNum = simpleNode->m_keyValue.first.operandValue().value;

		if (!leftIsSimple && simpleRuleIsNumber && simpleNodeNum == 0.0) {
			throw ExpressionTreeException("Division by zero", 0);
		}
	}

	return node;
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

	parent->m_left = unknownNode;
	parent->m_right = newRightNode;

	return parent;
}

EBST::NodePtr EBST::applyRule2ToSubTree(NodePtr& parent) const {
	auto& left = parent->m_left;
	auto& right = parent->m_right;

	// ((n * A) * B) -> (n * (A * B))

	const auto leftIsNumberAndSubtreeMul = getRuleForSubtree(left) == EBST::NodeRule::NumberAndSubtreeMul;
	auto& numberAndSubtreeMulNode = leftIsNumberAndSubtreeMul ? left : right;
	auto& subtreeBNode = leftIsNumberAndSubtreeMul ? right : left;

	const auto leftIsNumber = getRuleForSubtree(numberAndSubtreeMulNode->m_left) == EBST::NodeRule::NumberVar;
	auto& numberNode = leftIsNumber ? numberAndSubtreeMulNode->m_left : numberAndSubtreeMulNode->m_right;
	auto& subtreeANode = leftIsNumber ? numberAndSubtreeMulNode->m_right : numberAndSubtreeMulNode->m_left;

	auto newRightNode = allocateNode(ExpressionNode(OperatorType::Multiplication));
	newRightNode->m_left = subtreeANode;
	newRightNode->m_right = subtreeBNode;

	parent->m_left = numberNode;
	parent->m_right = newRightNode;

	return parent;
}

EBST::NodePtr EBST::applyRule3ToSubTree(NodePtr& parent) const {
	auto& left = parent->m_left;
	auto& right = parent->m_right;

	// ((x * A) +- (x * B)) -> (x * (A +- B))

	const auto parentOp = parent->m_keyValue.first;
	parent->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);
	
	const auto leftLeftIsUnknown = getRuleForSubtree(left->m_left) == EBST::NodeRule::UnknownVar;
	auto& leftUnknownNode = leftLeftIsUnknown ? left->m_left : left->m_right;
	auto& leftSubtreeNode = leftLeftIsUnknown ? left->m_right : left->m_left;

	const auto rightLeftIsUnknown = getRuleForSubtree(right->m_left) == EBST::NodeRule::UnknownVar;
	auto& rightSubtreeNode = rightLeftIsUnknown ? right->m_right : right->m_left;

	auto newRightNode = allocateNode(parentOp);
	newRightNode->m_left = leftSubtreeNode;
	newRightNode->m_right = rightSubtreeNode;

	parent->m_left = leftUnknownNode;
	parent->m_right = newRightNode;

	return parent;
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
    parent->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);

    auto newRightNode = allocateNode(parentOp);
	newRightNode->m_left = leftSubtreeNode;
	newRightNode->m_right = rightSubtreeNode;

	parent->m_left = leftNumberNode;
	parent->m_right = newRightNode;

	return parent;
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

	const auto leftIsUnknown = getRuleForSubtree(unknownAndSubtreeAddSubNode->m_left) == EBST::NodeRule::UnknownVar;
	auto& unknownNode = leftIsUnknown ? unknownAndSubtreeAddSubNode->m_left : unknownAndSubtreeAddSubNode->m_right;
	auto& subtreeBNode = leftIsUnknown ? unknownAndSubtreeAddSubNode->m_right : unknownAndSubtreeAddSubNode->m_left;

	auto newRightNode = allocateNode(subOp);
	newRightNode->m_left = subtreeANode;
	newRightNode->m_right = subtreeBNode;

	const auto isSubs = parentOp.operatorType() == OperatorType::Substitution;

	parent->m_keyValue.first = parentOp;
	parent->m_left = isSubs ? newRightNode : unknownNode;
	parent->m_right = isSubs ? unknownNode : newRightNode;

	return parent;
}

EBST::NodePtr EBST::applyRule6ToSubTree(NodePtr& parent) const {
	auto& left = parent->m_left;
	auto& right = parent->m_right;

	// (A +- (n +- B)) -> (n +- (A +- B))

	const auto parentOp = parent->m_keyValue.first;

	const auto leftIsNumberAndSubtreeAddSub = getRuleForSubtree(left) == EBST::NodeRule::NumberAndSubtreeAddSub;
	auto& numberAndSubtreeAddSubNode = leftIsNumberAndSubtreeAddSub ? left : right;
	auto& subtreeANode = leftIsNumberAndSubtreeAddSub ? right : left;
	const auto subOp = numberAndSubtreeAddSubNode->m_keyValue.first;

	const auto leftIsNumber = getRuleForSubtree(numberAndSubtreeAddSubNode->m_left) == EBST::NodeRule::NumberVar;
	auto& numberNode = leftIsNumber ? numberAndSubtreeAddSubNode->m_left : numberAndSubtreeAddSubNode->m_right;
	auto& subtreeBNode = leftIsNumber ? numberAndSubtreeAddSubNode->m_right : numberAndSubtreeAddSubNode->m_left;

	auto newRightNode = allocateNode(subOp);
	newRightNode->m_left = subtreeANode;
	newRightNode->m_right = subtreeBNode;

	const auto isSubs = parentOp.operatorType() == OperatorType::Substitution;

	parent->m_keyValue.first = parentOp;
	parent->m_left = isSubs ? newRightNode : numberNode;
	parent->m_right = isSubs ? numberNode : newRightNode;

	return parent;
}

bool EBST::subTreesAreEqual(const NodePtr& n1, const NodePtr& n2) const {
    if (!n1 || !n2 || !n1->m_left || !n2->m_left || !n1->m_right || !n2->m_right) {
        return false;
    }

	const auto getExpr = [](const NodePtr& ptr) {
	    return ptr->m_keyValue.first;
	};

	const auto parent1Expr = getExpr(n1);
    const auto parent2Expr = getExpr(n2);

    const auto left1Expr = getExpr(n1->m_left);
    const auto left2Expr = getExpr(n2->m_left);

    const auto right1Expr = getExpr(n1->m_right);
    const auto right2Expr = getExpr(n2->m_right);

    return parent1Expr == parent2Expr && left1Expr == left2Expr && right1Expr == right2Expr;
}

std::string EBST::outputInfix(const NodePtr& ptr, bool withBrackets) const {
    std::string result;

    if (ptr) {
		auto typeIsOperator = ptr->m_keyValue.first.type() == ExpressionType::Operator;

        if (withBrackets && typeIsOperator) {
            result += '(';
        }

        result += outputInfix(ptr->m_left, withBrackets);
        if (typeIsOperator) {
            result += ' ';
        }
        result += ptr->m_keyValue.first.toString();
        if (typeIsOperator) {
            result += ' ';
        }
        result += outputInfix(ptr->m_right, withBrackets);

        if (withBrackets && typeIsOperator) {
            result += ')';
        }
    }

    return result;
}

std::string EBST::outputPostfix(const EBST::NodePtr &ptr) const {
    std::string result;

    if (ptr) {
        result += outputPostfix(ptr->m_left);
        result += outputPostfix(ptr->m_right);
        result += ptr->m_keyValue.first.toString();
        result += ' ';
    }

    return result;
}

std::string EBST::outputPrefix(const EBST::NodePtr &ptr) const {
    std::string result;

    if (ptr) {
        result += ptr->m_keyValue.first.toString();
        result += ' ';
        result += outputPrefix(ptr->m_left);
        result += outputPrefix(ptr->m_right);
    }

    return result;
}

EBST::NodeRule operator|(EBST::NodeRule a, EBST::NodeRule b) {
	auto lhsNum = static_cast<int>(a);
	auto rhsNum = static_cast<int>(b);
	return static_cast<EBST::NodeRule>(lhsNum | rhsNum);
}

std::string EBST::toString(const NodeRule rule) const {
	std::map<NodeRule, std::string> ruleToStringMap {
		{ NodeRule::Subtree, "Subtree" },
		{ NodeRule::UnknownVar, "UnknownVar" },
		{ NodeRule::NumberVar, "NumberVar" },
		{ NodeRule::Multiplication, "Multiplication" },
		{ NodeRule::AdditionSubstitution, "AdditionSubstitution" },
		{ NodeRule::DivisionModulo, "DivisionModulo" },
		{ NodeRule::Power, "Power" },
		{ NodeRule::NoRule, "NoRule" },
		{ NodeRule::UnknownAndNumberMul, "UnknownAndNumberMul" },
		{ NodeRule::UnknownAndNumberAddSub, "UnknownAndNumberAddSub" },
		{ NodeRule::UnknownAndNumberDivMod, "UnknownAndNumberDivMod" },
		{ NodeRule::UnknownAndNumberPow, "UnknownAndNumberPow" },
		{ NodeRule::UnknownAndSubtree, "UnknownAndSubtree" },
		{ NodeRule::NumberAndSubtree, "NumberAndSubtree" },
		{ NodeRule::UnknownAndSubtreeMul, "UnknownAndSubtreeMul" },
		{ NodeRule::UnknownAndSubtreeAddSub, "UnknownAndSubtreeAddSub" },
		{ NodeRule::NumberAndSubtreeMul, "NumberAndSubtreeMul" },
		{ NodeRule::NumberAndSubtreeAddSub, "NumberAndSubtreeAddSub" },
		{ NodeRule::Rule1, "Rule1" },
		{ NodeRule::Rule2, "Rule2" },
		{ NodeRule::Rule3, "Rule3" },
		{ NodeRule::Rule4, "Rule4" },
		{ NodeRule::Rule5, "Rule5" },
		{ NodeRule::Rule6, "Rule6" }
	};

	const auto it = ruleToStringMap.find(rule);
	if (it != ruleToStringMap.end()) {
		return ruleToStringMap.at(rule);
	}

	return {};
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

// unused stuff

void EBST::insert(const ExpressionNode&, const bool &) {}

EBST::NodePtr EBST::insert(EBST::NodePtr&, const AbstractBST::KVPair&) {
    return EBST::NodePtr();
}

bool EBST::remove(const ExpressionNode&) {
    return false;
}

EBST::NodePtr EBST::remove(EBST::NodePtr&, const ExpressionNode&) {
    return EBST::NodePtr();
}

EBST::NodePtr EBST::find(const EBST::NodePtr&, const ExpressionNode&) const {
	return EBST::NodePtr();
}

EBST::iterator EBST::find(const ExpressionNode&) const {
	return iterator();
}

EBST::iterator EBST::begin() const {
	return AbstractBaseTree::begin();
}

EBST::Node::Node(const KVPair& keyValue) : AbstractNode(keyValue) {}

EBST::NodePtr EBST::Node::next() const {
    return NodePtr();
}
