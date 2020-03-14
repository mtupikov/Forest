#include "ExpressionNode.h"

#include "ExpressionException.h"

#include <string>
#include <cmath>
#include <iostream>

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

bool ExpressionNode::operator==(const ExpressionNode& rhs) const {
    if (m_type != rhs.m_type) {
        return false;
    }

    if (m_type == ExpressionType::Operator) {
        return operatorType() == rhs.operatorType();
    }

    if (isOperandUnknown(operandValue()) && isOperandUnknown(rhs.operandValue())) {
        return operandValue().variableName == rhs.operandValue().variableName;
    } else if (!isOperandUnknown(operandValue()) && !isOperandUnknown(rhs.operandValue())) {
        return operandValue().value == rhs.operandValue().value;
    }

    return false;
}

std::ostream& operator<<(std::ostream& os, const ExpressionNode& node) {
    os << node.toString();
    return os;
}

ExpressionNode ExpressionNode::operator*(const ExpressionNode& rhs) const {
    return ExpressionNode(operandValue().value * rhs.operandValue().value);
}

ExpressionNode ExpressionNode::operator/(const ExpressionNode& rhs) const {
	if (rhs.operandValue().value == 0.0) {
		throw ExpressionException(ExpressionError::DivisionByZero, 0);
	}

    return ExpressionNode(operandValue().value / rhs.operandValue().value);
}

ExpressionNode ExpressionNode::operator+(const ExpressionNode& rhs) const {
    return ExpressionNode(operandValue().value + rhs.operandValue().value);
}

ExpressionNode ExpressionNode::operator-(const ExpressionNode& rhs) const {
    return ExpressionNode(operandValue().value - rhs.operandValue().value);
}

ExpressionNode ExpressionNode::operator%(const ExpressionNode& rhs) const {
    auto result = std::remainder(operandValue().value, rhs.operandValue().value);
    std::cout << operandValue().value << " " << rhs.operandValue().value << std::endl;
    std::cout << result << std::endl;
    return ExpressionNode(result);
}

ExpressionNode ExpressionNode::operator^(const ExpressionNode& rhs) const {
    auto result = std::pow(operandValue().value, rhs.operandValue().value);
    return ExpressionNode(result);
}

std::string ExpressionNode::toString() const {
    if (m_type == ExpressionType::Operand) {
        if (isOperandUnknown(m_operand)) {
            return { m_operand.variableName };
        }

		return trimToStringDouble(m_operand.value);
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

bool isOperator(const ExpressionNode& node) {
	return node.type() == ExpressionType ::Operator;
}

bool isBracket(const ExpressionNode &ex) {
    const auto type = ex.operatorType();
    return type == OperatorType::BracketLeft || type == OperatorType ::BracketRight;
}

std::string trimToStringDouble(double d) {
	// lol
	if (d == 0.0) {
		d = abs(d);
	}

	auto str = std::to_string(d);

	auto lastZeroPos = str.find_last_not_of('0');
	if (lastZeroPos + 1 != str.length()) {
		lastZeroPos += 2;
		str.erase(lastZeroPos, std::string::npos);
	}

	return str;
}
