#include "EBST.h"

#include <cctype>
#include <stack>
#include <iostream>

EBST::EBST(const std::string& expressionString) {
    auto parsedExp = parseExpression(expressionString);
    std::stack<EBST::NodePtr> stack;
    EBST::NodePtr t;

    for (const auto& node : parsedExp) {
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

std::string EBST::toString(OutputType type) const {
    switch (type) {
    case OutputType::WithParenthese:
    case OutputType::Infix: return outputInfix(m_rootNode, (type & OutputType::WithParenthese) != 0);
    case OutputType::Postfix: return outputPostfix(m_rootNode);
    case OutputType::Prefix: return outputPrefix(m_rootNode);
    }

    return {};
}

std::vector<ExpressionResult> EBST::calculateResult() const {
    return std::vector<ExpressionResult>();
}

void EBST::insert(const ExpressionNode& key, const bool &) {

}

EBST::NodePtr EBST::insert(EBST::NodePtr& node, const AbstractBST::KVPair& keyValue) {
    return EBST::NodePtr();
}

bool EBST::remove(const ExpressionNode& key) {
    return false;
}

EBST::NodePtr EBST::remove(EBST::NodePtr& p, const ExpressionNode& key) {
    return EBST::NodePtr();
}

EBST::NodePtr EBST::find(const EBST::NodePtr& node, const ExpressionNode& key) const {
    return EBST::NodePtr();
}

void EBST::buildTree(const std::vector<ExpressionNode>& expr) {

}

std::vector<ExpressionNode> EBST::parseExpression(const std::string& expr) const {
    std::vector<ExpressionNode> output;
    std::stack<ExpressionNode> stack;

    const auto readToken = [&expr](auto& it) -> std::optional<ExpressionNode> {
        auto dotFound = false;

        if (std::isdigit(*it)) {
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

            assert(strVal.length() == 1 && "unknown operand must be 1 character");

            return parseOperandNodeFromString(strVal);
        }

        assert(false && "invalid token");
        return std::nullopt;
    };

    for (auto it = expr.cbegin(); it < expr.cend(); ++it) {
        auto c = *it;

        if (std::isspace(c)) {
            continue;
        }

        auto pOp = parseOperatorNodeFromChar(c);

        if (pOp.has_value() && !isBracket(pOp.value())) {
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
            auto pOpBr = parseOperatorNodeFromChar(c);
            if (pOpBr.has_value()) {
                stack.push(pOpBr.value());
            } else {
                assert(false && "dumb left bracket");
            }
        } else if (c == ')') {
            auto top = stack.top();
            while (stack.top().operatorType() != OperatorType::BracketLeft) {
                output.push_back(top);
                stack.pop();
                if (stack.empty()) {
                    assert(false && "missed '(' bracket");
                    break;
                }
                top = stack.top();
            }

            if (!stack.empty() && stack.top().operatorType() == OperatorType::BracketLeft) {
                stack.pop();
            }
        } else {
            auto pOpTok = readToken(it);
            if (pOpTok.has_value()) {
                output.push_back(pOpTok.value());
            } else {
                assert(false && "operand dumb");
            }
        }
    }

    while (!stack.empty()) {
        auto rToken = stack.top();
        if (isBracket(rToken)) {
            assert(false && "missed ')' bracket");
        }
        output.push_back(rToken);
        stack.pop();
    }

    return output;
}

std::string EBST::outputInfix(const NodePtr& ptr, bool withBrackets) const {
    std::string result;

    if (ptr) {
        if (withBrackets) {
            result += '(';
        }

        result += outputInfix(ptr->m_left, withBrackets);
        result += ptr->m_keyValue.first.toString();
        result += ' ';
        result += outputInfix(ptr->m_right, withBrackets);

        if (withBrackets) {
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

EBST::Node::Node(const KVPair& keyValue) : AbstractNode(keyValue) {}

EBST::NodePtr EBST::Node::next() const {
    return NodePtr();
}
