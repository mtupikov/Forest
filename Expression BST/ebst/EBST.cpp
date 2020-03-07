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
	// std::cout << "NodeRule::Subtree " << NodeRule::Subtree << std::endl;
	// std::cout << "NodeRule::UnknownVar " << NodeRule::UnknownVar << std::endl;
	// std::cout << "NodeRule::NumberVar " << NodeRule::NumberVar << std::endl;
	// std::cout << "NodeRule::UnknownAndSubtree " << NodeRule::UnknownAndSubtree << std::endl;
	// std::cout << "NodeRule::NumberAndSubtree " << NodeRule::NumberAndSubtree << std::endl;
	// std::cout << "NodeRule::Multiplication " << NodeRule::Multiplication << std::endl;
	// std::cout << "NodeRule::AdditionSubstitution " << NodeRule::AdditionSubstitution << std::endl;
	// std::cout << "NodeRule::NoRule " << NodeRule::NoRule << std::endl;
	// std::cout << "> NodeRule::UnknownAndSubtreeMul " << NodeRule::UnknownAndSubtreeMul << std::endl;
	// std::cout << "NodeRule::UnknownAndSubtreeAddSub " << NodeRule::UnknownAndSubtreeAddSub << std::endl;
	// std::cout << "NodeRule::NumberAndSubtreeMul " << NodeRule::NumberAndSubtreeMul << std::endl;
	// std::cout << "NodeRule::NumberAndSubtreeAddSub " << NodeRule::NumberAndSubtreeAddSub << std::endl;
	// std::cout << "NodeRule::Rule1 " << NodeRule::Rule1 << std::endl;
	// std::cout << "NodeRule::Rule2 " << NodeRule::Rule2 << std::endl;
	// std::cout << "NodeRule::Rule3 " << NodeRule::Rule3 << std::endl;
	// std::cout << "NodeRule::Rule4 " << NodeRule::Rule4 << std::endl;
	// std::cout << "NodeRule::Rule5 " << NodeRule::Rule5 << std::endl;
	// std::cout << "NodeRule::Rule6 " << NodeRule::Rule6 << std::endl;

    auto parsedExp = parseExpression(expressionString);
    buildTree(parsedExp);
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
	const auto leftRule = getRuleForSubtree(left);
	const auto rightRule = getRuleForSubtree(right);

	// if (nodeRule != NodeRule::NoRule && leftRule != NodeRule::NoRule && rightRule != NodeRule::NoRule) {
	// 	std::cout << "node rule: " << nodeRule << std::endl;
	// 	std::cout << "left rule: " << leftRule << std::endl;
	// 	std::cout << "right rule: " << rightRule << std::endl;
	// }

	const auto resultRule = validateRules(nodeRule, leftRule, rightRule);

	// std::cout << (nodeRule | leftRule | rightRule) << " -> " << resultRule << std::endl;

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

EBST::NodePtr EBST::reduceNode(const EBST::NodePtr &parent) {
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
			return calculateTwoNumbers(newNode, leftExp, rightExp);
		} else if (numberAndOperator) {
			auto& num = leftExprIsOperator ? rightExp : leftExp;
			return evaluateOperatorAndNumber(newNode, num, leftExprIsOperator);
		} else if (nodeHasUnknownExpr(newNode)) {
			return evaluateSubTreeWithUnknowns(newNode);
        }

		return newNode;
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

EBST::NodePtr EBST::applyRulesToSubTree(NodePtr& parent) const {
	const auto rule = getRuleForSubtree(parent);

	std::cout << outputInfix(parent, true) << " : " << rule << std::endl;

	// {
	// 	new_rule = 0;
	// 	rule_number;
	// 	if (rule & 0xffff0000)
	// 		{
	// 			rule_number = (rule & 0xffff0000) >> 16;
	// 			if (rule_number == 1) then Rule1
	// 			if (rule_number == 3) then Rule2
	// 		}
	// }

	switch (rule) {
	case NodeRule::Rule1: {
		std::cout << "rule 1" << std::endl;
		break;
	}
	case NodeRule::Rule2: {
		std::cout << "rule 2" << std::endl;

		break;
	}
	case NodeRule::Rule3: {
		std::cout << "rule 3" << std::endl;

		break;
	}
	case NodeRule::Rule4: {
		std::cout << "rule 4" << std::endl;

		break;
	}
	case NodeRule::Rule5: {
		std::cout << "rule 5" << std::endl;

		break;
	}
	case NodeRule::Rule6: {
		std::cout << "rule 6" << std::endl;

		break;
	}
	default: return parent;
	}

	return parent;
}

bool EBST::subTreeIsUnknownWithNumber(const NodePtr& node) const {
	auto& left = node->m_left;
	auto& right = node->m_right;

	if (!nodeHasChildren(node)) {
		return false;
	}

	auto leftExp = getExpressionNode(left);
	auto rightExp = getExpressionNode(right);

	const auto leftExprIsOperator = isOperator(leftExp);
	const auto leftExprIsUnknownOperand = !leftExprIsOperator && isOperandUnknown(leftExp.operandValue());
	const auto rightExprIsOperator = isOperator(rightExp);
	const auto rightExprIsUnknownOperand = !rightExprIsOperator && isOperandUnknown(rightExp.operandValue());

	return leftExprIsUnknownOperand ^ rightExprIsUnknownOperand;
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
EBST::NodeRule EBST::validateRules(const NodeRule rule1, const NodeRule rule2, const NodeRule rule3) const {
	const std::vector<NodeRule> allowedRulesOr {
		NodeRule::Subtree,
		NodeRule::UnknownVar,
		NodeRule::NumberVar,
		NodeRule::UnknownAndSubtree,
		NodeRule::NumberAndSubtree,
		NodeRule::Multiplication,
		NodeRule::AdditionSubstitution,
		NodeRule::UnknownAndSubtreeMul,
		NodeRule::UnknownAndSubtreeAddSub,
		NodeRule::NumberAndSubtreeMul,
		NodeRule::NumberAndSubtreeAddSub,
	};

	const std::vector<NodeRule> allowedRulesMul {
		NodeRule::Rule1,
		NodeRule::Rule2,
		NodeRule::Rule3,
		NodeRule::Rule4,
		NodeRule::Rule5,
		NodeRule::Rule6,
	};

	const auto ruleMul = static_cast<NodeRule>(rule1 * rule2 * rule3);
	const auto itMul = std::find_if(allowedRulesMul.cbegin(), allowedRulesMul.cend(), [ruleMul](const NodeRule r) {
		return ruleMul == r;
	});

	if (itMul != allowedRulesMul.cend()) {
		return ruleMul;
	}

	const auto ruleOr = rule1 | rule2 | rule3;
	const auto it = std::find_if(allowedRulesOr.cbegin(), allowedRulesOr.cend(), [ruleOr](const NodeRule r) {
		return r == ruleOr;
	});

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
