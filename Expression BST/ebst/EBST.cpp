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
