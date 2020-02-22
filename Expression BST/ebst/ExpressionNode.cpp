#include "ExpressionNode.h"

#include <string>

namespace {

const std::string variableRegex{"^([[:alpha:]])$"};
const std::string numberRegex{"^[+-]?(?=[.]?[0-9])[0-9]*(?:[.][0-9]*)?(?:[Ee][+-]?[0-9]+)?$"};

} // end anonymous namespace

ExpressionNode::ExpressionNode(OperatorType type) : ExpressionNode(ExpressionType::Operator) {
	m_operatorType = type;
}

ExpressionNode::ExpressionNode(double value) : ExpressionNode(ExpressionType::Operand) {
	m_operand.value = value;
}

ExpressionNode::ExpressionNode(char variableName) : ExpressionNode(ExpressionType::Operand) {
	m_operand.variableName = variableName;
}

ExpressionNode::ExpressionNode(ExpressionType type) : m_type(type) {}

ExpressionNode::ExpressionNode() {}

ExpressionType ExpressionNode::type() const {
	return m_type;
}

OperatorType ExpressionNode::operatorType() const {
	return m_operatorType;
}

Operand ExpressionNode::operandValue() const {
	return m_operand;
}

int ExpressionNode::operatorPrecedence() const {
    const std::map<OperatorType, int> operatorToPrecedence {
        { OperatorType::Addition, 2 },
        { OperatorType::Substitution, 2 },
        { OperatorType::Division, 3 },
        { OperatorType::Multiplication, 3 },
        { OperatorType::Modulo, 3 },
        { OperatorType::Power, 4 },
        { OperatorType::BracketLeft, 5 },
        { OperatorType::BracketRight, 5 },
    };

    const auto it = operatorToPrecedence.find(m_operatorType);
    if (it != operatorToPrecedence.cend()) {
        return operatorToPrecedence.at(m_operatorType);
    }

    assert(false && "dumb");
    return -1;
}

bool ExpressionNode::operator<(const ExpressionNode& rhs) const {
    return operatorPrecedence() < rhs.operatorPrecedence();
}

bool ExpressionNode::operator>(const ExpressionNode& rhs) const {
    return rhs < *this;
}

bool ExpressionNode::operator<=(const ExpressionNode& rhs) const {
    return !(rhs < *this);
}

bool ExpressionNode::operator>=(const ExpressionNode& rhs) const {
    return !(*this < rhs);
}

std::ostream& operator<<(std::ostream& os, const ExpressionNode& node) {
    os << node.toString();
    return os;
}

std::string ExpressionNode::toString() const {
    if (m_type == ExpressionType::Operand) {
        if (isOperandUnknown(m_operand)) {
            return { m_operand.variableName };
        }

        return std::to_string(m_operand.value);
    }

    const std::map<OperatorType, char> operatorToChar {
        { OperatorType::Addition, '+' },
        { OperatorType::Substitution, '-' },
        { OperatorType::Division, '/' },
        { OperatorType::Multiplication, '*' },
        { OperatorType::Modulo, '%' },
        { OperatorType::Power, '^' },
        { OperatorType::BracketLeft, '(' },
        { OperatorType::BracketRight, ')' }
    };

    const auto it = operatorToChar.find(m_operatorType);
    if (it != operatorToChar.cend()) {
        return { operatorToChar.at(m_operatorType) };
    }

    assert(false && "invalid expression node");
    return {};
}

std::optional<ExpressionNode> parseOperandNodeFromString(const std::string &str) {
   std::smatch sm;
   if (std::regex_match(str, sm, std::regex(variableRegex))) {
	   assert(sm.size() == 2);

	   const auto varName = sm[1].str();
	   assert(varName.length() == 1);

	   return ExpressionNode(varName[0]);
   } else if (std::regex_match(str, sm, std::regex(numberRegex))) {
	   assert(!sm.empty());

	   const auto strNum = sm[0].str();
	   const auto num = std::stod(strNum);
	   return ExpressionNode(num);
   }

   return std::nullopt;
}

std::optional<ExpressionNode> parseOperatorNodeFromChar(char op) {
	const std::map<char, OperatorType> charToOperandType {
		{ '+', OperatorType::Addition },
		{ '-', OperatorType::Substitution },
		{ '/', OperatorType::Division },
		{ '*', OperatorType::Multiplication },
		{ '%', OperatorType::Modulo },
		{ '^', OperatorType::Power },
        { '(', OperatorType::BracketLeft },
        { ')', OperatorType::BracketRight }
	};

	const auto it = charToOperandType.find(op);
	if (it != charToOperandType.cend()) {
		return ExpressionNode(charToOperandType.at(op));
	}

	return std::nullopt;
}

bool isOperandUnknown(const Operand& op) {
	return op.variableName != invalidOperandVarName;
}

bool isOperator(char c) {
    return parseOperatorNodeFromChar(c).has_value();
}

bool isBracket(const ExpressionNode &ex) {
    const auto type = ex.operatorType();
    return type == OperatorType::BracketLeft || type == OperatorType ::BracketRight;
}
