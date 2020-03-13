#include "EBST.h"

#include "ExpressionException.h"

#include <cctype>
#include <stack>
#include <exception>
#include <algorithm>
#include <set>

EBST::EBST(const std::string& expressionString) {
    auto parsedExp = parseExpression(expressionString);
    buildTree(parsedExp);
	m_reducedTreeRootNode = buildReducedFormTree(m_rootNode);
	m_balancedTreeRootNode = buildBalancedTree(m_reducedTreeRootNode);

	if (!treeIsBalanced()) {
		m_reducedTreeRootNode = buildReducedFormTree(m_balancedTreeRootNode);
		m_balancedTreeRootNode = buildBalancedTree(m_reducedTreeRootNode);

		if (!treeIsBalanced()) {
			throw ExpressionException(ExpressionError::CannotBalance, 0);
		}
	}

	solveExpression();
}

std::string EBST::toString(OutputType type) const {
    switch (type) {
    case OutputType::InfixWithParentheses:
    case OutputType::Infix: return outputInfix(m_rootNode, type == OutputType::InfixWithParentheses);
    case OutputType::Postfix: return outputPostfix(m_rootNode);
    case OutputType::Prefix: return outputPrefix(m_rootNode);
    case OutputType::ReducedInfixWithParentheses:
	case OutputType::ReducedInfix: return outputInfix(m_balancedTreeRootNode, type == OutputType::ReducedInfixWithParentheses);
    }

	return {};
}

int EBST::maxDegree() const {
	return m_maxDegree;
}

std::string EBST::unknownOperandName() const {
	return m_unknownOperandName;
}

ExpressionSolution EBST::solution() const {
	return m_solution;
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

std::vector<ExpressionNode> EBST::parseExpression(const std::string& expr) {
    std::vector<ExpressionNode> output;
    std::stack<ExpressionNode> stack;
	std::set<char> unknownOperands;

	const auto readToken = [&expr, &unknownOperands, &stack](auto& it, const std::optional<ExpressionNode>& lastExpr) -> std::optional<ExpressionNode> {
        auto dotFound = false;

        if (std::isdigit(*it) || *it == '-' || *it == '+') {
            auto begin = it;

			if (lastExpr.has_value() && *it == '-'
			    && (lastExpr.value().operatorType() == OperatorType::Substitution || lastExpr.value().operatorType() == OperatorType::Addition)) {
				auto lastOp = lastExpr.value().operatorType();

				if (lastOp == OperatorType::Substitution) {
					stack.pop();
					stack.push(ExpressionNode(OperatorType::Addition));
					++begin;
				} else if (lastOp == OperatorType::Addition) {
					stack.pop();
					stack.push(ExpressionNode(OperatorType::Substitution));
					++begin;
				}
			}

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
				throw ExpressionException(ExpressionError::UnknownOperandSize, static_cast<int>(std::distance(expr.cbegin(), it)));
            }

			const auto ch = static_cast<char>(std::tolower(strVal.front()));
			unknownOperands.insert(ch);
			if (unknownOperands.size() > 1) {
				throw ExpressionException(ExpressionError::MultipleUnknownOperands, static_cast<int>(std::distance(expr.cbegin(), it)));
			}

			return parseOperandNodeFromString({ ch });
        }

		throw ExpressionException(ExpressionError::InvalidToken, static_cast<int>(std::distance(expr.cbegin(), it)));
        return std::nullopt;
    };

    const auto nextTokenIsNumber = [&expr](auto it) {
        const auto next = std::next(it, 1);
        return next != expr.cend() && std::isdigit(*next);
    };

	const auto expressionNodeIsOperator = [](const std::optional<ExpressionNode>& op) {
		return (op.has_value() && op.value().type() == ExpressionType::Operator) || !op.has_value();
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
				throw ExpressionException(ExpressionError::OperatorAfterOperator, distanceFromBegin(it));
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
				throw ExpressionException(ExpressionError::LeftBracketError, distanceFromBegin(it));
            }
        } else if (c == ')') {
            auto top = stack.top();
            while (stack.top().operatorType() != OperatorType::BracketLeft) {
                output.push_back(top);
                stack.pop();
                if (stack.empty()) {
					throw ExpressionException(ExpressionError::MissingLeftParentheses, distanceFromBegin(it));
                }
                top = stack.top();
            }

            if (!stack.empty() && stack.top().operatorType() == OperatorType::BracketLeft) {
                stack.pop();
            }
        } else {
            if (lastExpressionNode.has_value() && lastExpressionNode.value().type() == ExpressionType::Operand) {
				throw ExpressionException(ExpressionError::MissingOperator, distanceFromBegin(it));
            }

			pOp = readToken(it, lastExpressionNode);
            if (pOp.has_value()) {
                output.push_back(pOp.value());
            } else {
				throw ExpressionException(ExpressionError::InvalidToken, distanceFromBegin(it));
            }
        }

        lastExpressionNode = pOp;
    }

    while (!stack.empty()) {
        auto rToken = stack.top();
        if (isBracket(rToken)) {
			throw ExpressionException(ExpressionError::MissingRightParentheses, distanceFromBegin(expr.cend()));
        }
        output.push_back(rToken);
        stack.pop();
    }

	assert(unknownOperands.size() <= 1 && "missed multiple unknown operands");
	if (unknownOperands.size() == 1) {
		m_unknownOperandName = static_cast<char>(*unknownOperands.cbegin());
	}

	return output;
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
