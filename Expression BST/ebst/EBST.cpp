#include "EBST.h"

#include <cctype>
#include <stack>
#include <exception>

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

void EBST::buildReducedFormTree() {
    m_reducedTreeRootNode = reduceNode(m_rootNode);
}

EBST::NodePtr EBST::reduceNode(const EBST::NodePtr &parent) {
    const auto getExpressionNode = [](const NodePtr& ptr) {
        return ptr->m_keyValue.first;
    };

    const auto isOperator = [](const ExpressionNode& node) {
        return node.type() == ExpressionType ::Operator;
    };

    const auto allocateNode = [](const ExpressionNode& node) {
        auto pair = std::make_pair(node, false);
        return std::make_shared<EBST::Node>(pair);
    };

    const auto calculateTwoNumbers = [&](const NodePtr& node, const ExpressionNode& leftExp, const ExpressionNode& rightExp) {
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
    };

    const auto nodeHasUnknownExpr = [&getExpressionNode, &isOperator](const NodePtr& ptr) {
        auto left = ptr->m_left;
        auto right = ptr->m_right;

        if (!left || !right) {
            return false;
        }

        const auto leftExpr = getExpressionNode(left);
        const auto rightExpr = getExpressionNode(right);
        const auto operandsUnknown = isOperandUnknown(leftExpr.operandValue()) || isOperandUnknown(rightExpr.operandValue());
        const auto areOperators = isOperator(leftExpr) || isOperator(rightExpr);
        return operandsUnknown && !areOperators;
    };

    const auto evaluateSubTreeWithUnknowns = [&getExpressionNode](const NodePtr& ptr) {
        auto& left = ptr->m_left;
        auto& right = ptr->m_right;

        const auto parentExpr = getExpressionNode(ptr);
        const auto leftExpr = getExpressionNode(left);
        const auto rightExpr = getExpressionNode(right);
        const auto leftIsUnknown = isOperandUnknown(leftExpr.operandValue());
        const auto rightIsUnknown = isOperandUnknown(rightExpr.operandValue());

        const auto resetLeftRight = [&left, &right] {
            left.reset();
            right.reset();
        };

        if (leftIsUnknown && rightIsUnknown) {
            switch (parentExpr.operatorType()) {
            case OperatorType::Substitution: {
                resetLeftRight();
                ptr->m_keyValue.first = ExpressionNode(0.0);
                break;
            }
            case OperatorType::Addition: {
                ptr->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);
                right->m_keyValue.first = ExpressionNode(2.0);
                break;
            }
            case OperatorType::Multiplication: {
                ptr->m_keyValue.first = ExpressionNode(OperatorType::Power);
                right->m_keyValue.first = ExpressionNode(2.0);
                break;
            }
            case OperatorType::Division: {
                resetLeftRight();
                ptr->m_keyValue.first = ExpressionNode(1.0);
                break;
            }
            case OperatorType::Modulo: {
                resetLeftRight();
                ptr->m_keyValue.first = ExpressionNode(1.0);
                break;
            }
            case OperatorType::Power: break;
            default: assert(false && "invalid operator");
            }
        } else {
            switch (parentExpr.operatorType()) {
                case OperatorType::Substitution: {
                    if (leftIsUnknown && rightExpr.operandValue().value == 0.0) {
                        resetLeftRight();
                        ptr->m_keyValue.first = ExpressionNode(leftExpr);
                    } else if (rightIsUnknown && leftExpr.operandValue().value == 0.0) {
                        ptr->m_keyValue.first = ExpressionNode(OperatorType::Multiplication);
                        left->m_keyValue.first = ExpressionNode(-1.0);
                        right->m_keyValue.first = rightExpr;
                    }
                    break;
                }
                case OperatorType::Addition: {
                    if ((leftIsUnknown && rightExpr.operandValue().value == 0.0) || (rightIsUnknown && leftExpr.operandValue().value == 0.0)) {
                        resetLeftRight();
                        ptr->m_keyValue.first = ExpressionNode(leftIsUnknown ? leftExpr : rightExpr);
                    }
                    break;
                }
                case OperatorType::Multiplication: {
                    if ((leftIsUnknown && rightExpr.operandValue().value == 0.0) || (rightIsUnknown && leftExpr.operandValue().value == 0.0)) {
                        resetLeftRight();
                        ptr->m_keyValue.first = ExpressionNode(0.0);
                    }
                    break;
                }
                case OperatorType::Division: {
                    if (leftIsUnknown && rightExpr.operandValue().value == 0.0) {
                        throw ExpressionTreeException("Division by zero", 0);
                    } else if (rightIsUnknown && leftExpr.operandValue().value == 0.0) {
                        resetLeftRight();
                        ptr->m_keyValue.first = ExpressionNode(0.0);
                    }
                    break;
                }
                case OperatorType::Modulo: {
                    if (leftIsUnknown && rightExpr.operandValue().value == 0.0) {
                        throw ExpressionTreeException("Division by zero", 0);
                    } else if (rightIsUnknown && leftExpr.operandValue().value == 0.0) {
                        resetLeftRight();
                        ptr->m_keyValue.first = ExpressionNode(0.0);
                    }
                    break;
                }
                case OperatorType::Power: {
                    if (leftIsUnknown && rightExpr.operandValue().value == 0.0) {
                        resetLeftRight();
                        ptr->m_keyValue.first = ExpressionNode(1.0);
                    } else if (rightIsUnknown && leftExpr.operandValue().value == 0.0) {
                        resetLeftRight();
                        ptr->m_keyValue.first = ExpressionNode(0.0);
                    }
                    break;
                }
                default: assert(false && "invalid operator");
            }
        }

        return ptr;
    };

    auto newNode = allocateNode(parent->m_keyValue.first);

    auto left = parent->m_left;
    auto right = parent->m_right;
    if (left && right) {
        newNode->m_left = reduceNode(left);
        newNode->m_right = reduceNode(right);

        auto leftExp = getExpressionNode(newNode->m_left);
        auto rightExp = getExpressionNode(newNode->m_right);

        if (!isOperator(leftExp)
            && !isOperandUnknown(leftExp.operandValue())
            && !isOperator(rightExp)
            && !isOperandUnknown(rightExp.operandValue())) {
            return calculateTwoNumbers(newNode, leftExp, rightExp);
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
                throw ExpressionTreeException("Unknown operand must be 1 character", std::distance(expr.cbegin(), it));
            }

            return parseOperandNodeFromString(strVal);
        }

        throw ExpressionTreeException("Invalid token", std::distance(expr.cbegin(), it));
        return std::nullopt;
    };

    const auto nextTokenIsNumber = [&expr](auto it) {
        const auto next = std::next(it, 1);
        return next != expr.cend() && std::isdigit(*next);
    };

    const auto lastExpressionNodeIsOperator = [](const std::optional<ExpressionNode>& op) {
        return op.has_value() && op.value().type() == ExpressionType::Operator;
    };

    std::optional<ExpressionNode> lastExpressionNode;
    for (auto it = expr.cbegin(); it < expr.cend(); ++it) {
        const auto c = *it;

        if (std::isspace(c)) {
            continue;
        }

        auto pOp = parseOperatorNodeFromChar(c);

        const auto nextTokenIsNumberAndPrevIsLeftBracket = nextTokenIsNumber(it)
                                                        && lastExpressionNodeIsOperator(lastExpressionNode);

        if (pOp.has_value() && !isBracket(pOp.value())
            && !nextTokenIsNumberAndPrevIsLeftBracket) {
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
                throw ExpressionTreeException("Left bracket is invalid", std::distance(expr.cbegin(), it));
            }
        } else if (c == ')') {
            auto top = stack.top();
            while (stack.top().operatorType() != OperatorType::BracketLeft) {
                output.push_back(top);
                stack.pop();
                if (stack.empty()) {
                    throw ExpressionTreeException("Missing '(' parentheses" , std::distance(expr.cbegin(), it));
                    break;
                }
                top = stack.top();
            }

            if (!stack.empty() && stack.top().operatorType() == OperatorType::BracketLeft) {
                stack.pop();
            }
        } else {
            std::optional<ExpressionNode> k;
            if (lastExpressionNode.has_value()) {
                k = lastExpressionNode.value();
                auto v = k.value();
            }
            if (lastExpressionNode.has_value() && lastExpressionNode.value().type() == ExpressionType::Operand) {
                throw ExpressionTreeException("Missing operator between operands" , std::distance(expr.cbegin(), it));
            }

            pOp = readToken(it);
            if (pOp.has_value()) {
                output.push_back(pOp.value());
            } else {
                throw ExpressionTreeException("Invalid operand" , std::distance(expr.cbegin(), it));
            }
        }

        lastExpressionNode = pOp;
    }

    while (!stack.empty()) {
        auto rToken = stack.top();
        if (isBracket(rToken)) {
            throw ExpressionTreeException("Missing ')' parentheses" , std::distance(expr.cbegin(), expr.cend()));
        }
        output.push_back(rToken);
        stack.pop();
    }

    return output;
}

std::string EBST::outputInfix(const NodePtr& ptr, bool withBrackets) const {
    std::string result;

    if (ptr) {
        auto typeIsOperator = ptr->m_keyValue.first.type() == ExpressionType ::Operator;

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

// unused stuff

void EBST::insert(const ExpressionNode& key, const bool &) {}

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

EBST::Node::Node(const KVPair& keyValue) : AbstractNode(keyValue) {}

EBST::NodePtr EBST::Node::next() const {
    return NodePtr();
}
