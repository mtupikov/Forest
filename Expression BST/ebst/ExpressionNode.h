#pragma once

#include <optional>
#include <type_traits>
#include <regex>
#include <assert.h>
#include <map>
#include <ostream>

const auto invalidOperandVarName = '?';

enum class OperatorType {
    Invalid,
	Addition,
	Substitution,
	Division,
	Multiplication,
	Modulo,
	Power,
	BracketLeft,
	BracketRight
};

enum class ExpressionType {
    Invalid,
	Operator,
	Operand
};

struct Operand final {
	double value = 0.0;
	char variableName = invalidOperandVarName;
};

class ExpressionNode {
public:
    ExpressionNode();
	explicit ExpressionNode(OperatorType type);
	explicit ExpressionNode(double value);
	explicit ExpressionNode(char variableName);

	ExpressionType type() const;
	OperatorType operatorType() const;
	Operand operandValue() const;
	int operatorPrecedence() const;

    bool operator<(const ExpressionNode& rhs) const;
    bool operator>(const ExpressionNode& rhs) const;
    bool operator<=(const ExpressionNode& rhs) const;
    bool operator>=(const ExpressionNode& rhs) const;
    bool operator==(const ExpressionNode& rhs) const;

    ExpressionNode operator*(const ExpressionNode& rhs) const;
    ExpressionNode operator/(const ExpressionNode& rhs) const;
    ExpressionNode operator+(const ExpressionNode& rhs) const;
    ExpressionNode operator-(const ExpressionNode& rhs) const;
    ExpressionNode operator%(const ExpressionNode& rhs) const;
    ExpressionNode operator^(const ExpressionNode& rhs) const;

    friend std::ostream &operator<<(std::ostream &os, const ExpressionNode &node);

    std::string toString() const;

private:
	ExpressionNode(ExpressionType type);

    OperatorType m_operatorType = OperatorType::Invalid;
    Operand m_operand;
	ExpressionType m_type = ExpressionType::Invalid;
};

std::optional<ExpressionNode> parseOperandNodeFromString(const std::string &str);
std::optional<ExpressionNode> parseOperatorNodeFromChar(char op);

bool isOperandUnknown(const Operand& op);
bool isOperator(char c);
bool isOperator(const ExpressionNode& node);
bool isBracket(const ExpressionNode& ex);
std::string trimToStringDouble(double d);
